#ifndef SPECTROGRAM_HPP
#define SPECTROGRAM_HPP

#include <vector>
#include <complex>
#include <utility>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <fftw3.h>
#include <omp.h>

namespace a2i {

  enum graphModes { LIN = 0, LOG = 1 };
  enum lineTypes { LINES = 0, BEZIE = 1, BARS = 2 };
  enum fillTypes { NOT_FILLED = 0, ONE_COLOR = 1, GRADIENT = 2 };
  enum windowFunctions {
    SINE=0, HANN, HAMMING, BLACKMAN,
    NUTTALL, BLACKMAN_NUTTALL,
    BLACKMAN_HARRIS, FLAT_TOP,
    BARTLETT_HANN, HANN_POISSON
  };

  class Spectrogram {
  public:
    Spectrogram();
    ~Spectrogram();

    void setSampleRate(
      unsigned int audio_sample_rate
    );
    void setDbRange(
      std::pair<int,int> audio_db_range = {-90,6}
    );
    void setFreqRange(
      std::pair<unsigned int,unsigned int> audio_freq_range = {0,20000}
    );
    void setFrameSize(int size);
    void setWindowFunc(int type);

    void addWindow();
    void fft();
    void normalize(int multiplier = 20);

    void drawGrid(
      cv::Mat& img,
      int type,
      bool enable_text = true,
      const std::vector<unsigned int>& freqs = {},
      int number_of_db_risks = 10,
      cv::Scalar line_color = cv::Scalar(79,73,80),
      cv::Scalar text_color = cv::Scalar(51,186,243)
    );
    void drawSpectrum(
      cv::Mat& img,
      int line_type = LINES,
      int graph_mode = LOG,
      int fill_type = ONE_COLOR,
      bool border_line = false,
      cv::Scalar line_color = cv::Scalar(255,255,255),
      cv::Scalar underline_color = cv::Scalar(127,127,127),
      int gradient_coefficient = 127
    );

    std::vector<double> out;
    std::vector<std::complex<double>> fft_out;
    std::vector<double> window_out;
    std::vector<double> in;

  private:
    fftw_plan               plan_r2c_;
    std::vector<double>       buffer_in_;    // length = frame_size
    std::vector<std::complex<double>> buffer_out_; // length = frame_size/2+1

    int                        frame_size_;
    unsigned int               sample_rate_;
    std::pair<int,int>         db_range_;
    std::pair<unsigned int,unsigned int> freq_range_;

    std::vector<double>        angle_full_;  // 2π*i/frame_size

    using WinFunc = void (Spectrogram::*)();
    std::vector<WinFunc>       windows_;

    void windowSine();
    void windowHann();
    void windowHamming();
    void windowBlackman();
    void windowNuttall();
    void windowBlackmanNuttall();
    void windowBlackmanHarris();
    void windowFlatTop();
    void windowBartlettHann();
    void windowHannPoisson();

    void drawGraientLine(
      cv::Mat& img,
      int fill_type,
      const cv::Point& p0,
      const cv::Point& p1,
      const cv::Scalar& underline_color,
      int gradient_color
    );

    double interpolate(double from, double to, double t);
  };

}

#endif // SPECTROGRAM_HPP
