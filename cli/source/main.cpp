#include "config.hpp"
#include "parser.hpp"
#include "version.hpp"

#include "raylib.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <a2i/spectrogram.hpp>

#include <thread>
#include <atomic>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <deque>
#include <vector>
#include <chrono>
#include <sys/select.h>

typedef struct {
    std::deque<cv::Mat> prev;    // теперь очередь для истории
    cv::Mat img;
    cv::Mat cur_img;
    cv::Mat grid;
} Window;

typedef struct {
    float left;
    float right;
} Frame;

// Глобальные переменные
Window w;
a2i::Spectrogram g;
std::pair<int,int> WINDOW;
unsigned int FRAME_SIZE;
int multiplier;
bool SHOW = false;
bool DEBUG_MODE;
bool ONLY_AUDIO;
bool MIC_MODE;

Config config;
std::atomic<bool> PAUSE(true);
std::atomic<bool> RUNNING(true);

// Терминал в raw‑режим один раз
static struct termios orig_termios;
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// kbhit через select (не блочит, немного жрёт CPU без sleep)
int kbhit() {
    fd_set set;
    struct timeval tv{0,0};
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    return select(STDIN_FILENO+1, &set, nullptr, nullptr, &tv) == 1;
}

// getch в raw‑режиме (блокирует)
int getch() {
    return getchar();
}

void callback(void* bufferData, unsigned int frames) {
    if (frames < 512) return;
    auto* fs = static_cast<Frame*>(bufferData);

    // сдвигаем старые выборки в кольце
    if (g.in.size() == FRAME_SIZE) {
        std::rotate(g.in.begin(), g.in.begin() + 512, g.in.end());
        g.in.resize(FRAME_SIZE - 512);
    }
    // добавляем новые
    for (unsigned int i = 0; i < 512; ++i) {
        g.in.push_back((fs[i].left + fs[i].right) * 0.5f);
    }

    if (g.in.size() == FRAME_SIZE) {
        g.addWindow();
        g.fft();
        g.normalize(multiplier);
        SHOW = true;
    } else {
        SHOW = false;
    }
}

void handleConsoleInput(Music &music) {
    while (RUNNING) {
        if (kbhit()) {
            int key = getch();
            switch (key) {
                case ' ':
                    if (PAUSE) PauseMusicStream(music);
                    else       ResumeMusicStream(music);
                    PAUSE = !PAUSE;
                    break;
                case 'q': case 'Q':
                    RUNNING = false;
                    return;
                case 'd': case 'D':
                    SeekMusicStream(music, GetMusicTimePlayed(music) + 5.0f);
                    break;
                case 'a': case 'A':
                    SeekMusicStream(music, GetMusicTimePlayed(music) - 5.0f);
                    break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool processKey(Music& music) {
  int key = cv::waitKey(1);

  if (key == ' ') {
      if (PAUSE) PauseMusicStream(music);
      else       ResumeMusicStream(music);
      PAUSE = !PAUSE;
  }
  if (key == 27 || key == 'q' || key == 'Q') {
      RUNNING = false;
      return false;
  }
  if (key == 'd' || key == 'D' || key == 83) {
      SeekMusicStream(music, GetMusicTimePlayed(music) + 5.0f);
  }
  if (key == 'a' || key == 'A' || key == 81) {
      SeekMusicStream(music, GetMusicTimePlayed(music) - 5.0f);
  }
  return true;
}

void processAudio(std::deque<cv::Mat>& history) {
    // фон
    w.img = cv::Mat(WINDOW.first, WINDOW.second, CV_8UC3, cv::Scalar(22,16,20));
    if (config.params.grid) cv::add(w.grid, w.img, w.img);

    // рисуем новый спектр
    w.cur_img = cv::Mat::zeros(WINDOW.first, WINDOW.second, CV_8UC3);
    g.drawSpectrum(
        w.cur_img,
        config.params.lineType,
        config.params.graphMode,
        config.params.fillType,
        config.params.border,
        config.params.lineColor,
        config.params.underlineColor,
        config.params.colormapCoef
    );

    // накопление предыдущих кадров через историю
    for (size_t i = 0; i < history.size(); ++i) {
        double alpha = 0.3 / (i + 1);
        cv::addWeighted(w.img, 1.0, history[i], alpha, 0.0, w.img);
    }
    cv::add(w.cur_img, w.img, w.img);

    // колоризация
    if (config.params.colormap >= 0)
        cv::applyColorMap(w.img, w.img, config.params.colormap);

    // показ
    cv::imshow("a2i", w.img);

    // обновляем историю (кольцевой буфер)
    if (history.size() == config.params.previousFrames)
        history.pop_back();
    history.push_front(w.cur_img.clone());
}

int main(int argc, char** argv) {
    Parser parser(argc, argv, config);
    if (config.end) return 0;
    if (!config.validate()) { std::cerr<<"Config error\n"; return 1; }
    if (!config.execute()) return 0;

    std::string file = config.audiofile;
    if (file.empty()) {
        std::cerr<<"Error: No audio file\n";
        parser.printUsage();
        return 0;
    }

    ONLY_AUDIO = config.params.onlyAudio;
    DEBUG_MODE  = config.params.debug;
    MIC_MODE    = (config.command=="mic");
    WINDOW      = config.params.windowSize;
    FRAME_SIZE  = config.params.framesize;
    multiplier  = config.params.normalizeMultiplier;

    if (!DEBUG_MODE)
        SetTraceLogLevel(LOG_WARNING);

    enableRawMode();  // raw‑режим терминала

    // инициализируем аудио
    InitAudioDevice();
    AudioStream stream{};
    Music music{};

    if (MIC_MODE && !ONLY_AUDIO) {
        stream = LoadAudioStream(44100,16,1);
        PlayAudioStream(stream);
    } else {
        music = LoadMusicStream(file.c_str());
        PlayMusicStream(music);
        SetMusicVolume(music, config.params.volume);
        if (!ONLY_AUDIO)
            AttachAudioStreamProcessor(music.stream, callback);
    }

    // GUI
    std::vector<Frame> audioBuffer(FRAME_SIZE);
    std::deque<cv::Mat> history;
    if (!ONLY_AUDIO) {
        g.setSampleRate(music.stream.sampleRate);
        g.setDbRange(config.params.amplitudeRange);
        g.setFreqRange({20,20000});
        g.setFrameSize(FRAME_SIZE);
        g.setWindowFunc(config.params.windowFunc);

        cv::namedWindow("a2i", cv::WINDOW_NORMAL);
        cv::resizeWindow("a2i", WINDOW.second, WINDOW.first);

        w.prev.clear();
        w.prev.resize(config.params.previousFrames);
        w.grid = cv::Mat::zeros(WINDOW.first, WINDOW.second, CV_8UC3);
        if (config.params.grid) {
            g.drawGrid(w.grid, config.params.graphMode, true,
                       {20,50,100,200,500,1000,2000,5000,10000,20000},
                       10,
                       config.params.gridLineColor,
                       config.params.gridTextColor);
        }
        cv::imshow("a2i", cv::Mat(WINDOW.first, WINDOW.second, CV_8UC3, cv::Scalar(22,16,20)));
        cv::waitKey(1);
    }

    // поток для консоли
    std::thread consoleThread(handleConsoleInput, std::ref(music));

    // главное окно- и аудио-циклы
    while (RUNNING) {
        if (MIC_MODE && !ONLY_AUDIO) {
            if (IsAudioStreamProcessed(stream)) {
                UpdateAudioStream(stream, audioBuffer.data(), FRAME_SIZE);
                callback(audioBuffer.data(), FRAME_SIZE);
            }
        } else {
            UpdateMusicStream(music);
            if (!processKey(music)) break;
        }

        if (SHOW && !ONLY_AUDIO) {
            processAudio(history);
        }

        // ограничение CPU / FPS (~60 fps)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    consoleThread.join();

    // очистка
    if (MIC_MODE && !ONLY_AUDIO) {
        StopAudioStream(stream);
        UnloadAudioStream(stream);
    } else {
        UnloadMusicStream(music);
    }
    CloseAudioDevice();
    disableRawMode();
    cv::destroyAllWindows();
    return 0;
}


