#include "spectrogram.hpp"
#include <iostream>
#include <sstream>

using namespace a2i;

Spectrogram::Spectrogram()
  : plan_r2c_(nullptr),
    frame_size_(0),
    sample_rate_(44100),
    db_range_{-90,6},
    freq_range_{0,20000},
    windows_{
      &Spectrogram::windowSine,
      &Spectrogram::windowHann,
      &Spectrogram::windowHamming,
      &Spectrogram::windowBlackman,
      &Spectrogram::windowNuttall,
      &Spectrogram::windowBlackmanNuttall,
      &Spectrogram::windowBlackmanHarris,
      &Spectrogram::windowFlatTop,
      &Spectrogram::windowBartlettHann,
      &Spectrogram::windowHannPoisson
    }
{
}

Spectrogram::~Spectrogram()
{
    if(plan_r2c_) {
        fftw_destroy_plan(plan_r2c_);
    }
}

void Spectrogram::setSampleRate(
  unsigned int audio_sample_rate)
{
    sample_rate_ = audio_sample_rate;
}

void Spectrogram::setDbRange(
  std::pair<int,int> audio_db_range)
{
    db_range_    = audio_db_range;
}

void Spectrogram::setFreqRange(
  std::pair<unsigned int,unsigned int> audio_freq_range)
{
    freq_range_ = audio_freq_range;
}

void Spectrogram::setFrameSize(int size)
{
    frame_size_ = size;
    out       .assign(size/2,      0.0f);
    fft_out.assign(size/2, std::complex<double>(0.0f, 0.0f));
    window_out.assign(size,        0.0f);
    in        .assign(size,        0.0f);

    buffer_in_ .assign(size, 0.0f);
    buffer_out_.assign(size/2 + 1, std::complex<double>(0.0f, 0.0f));

    angle_full_.resize(size);
    for(int i=0;i<size;++i) {
        angle_full_[i] = 2.0*M_PI * i / size;
    }

    if(plan_r2c_) fftw_destroy_plan(plan_r2c_);
    plan_r2c_ = fftw_plan_dft_r2c_1d(
      frame_size_,
      buffer_in_.data(),
      reinterpret_cast<fftw_complex*>(buffer_out_.data()),
      FFTW_MEASURE
    );
}

void Spectrogram::setWindowFunc(int type)
{
    (this->*windows_[type])();
}

void Spectrogram::windowSine()
{
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        // sin(π*i/(2*frame_size)) = sin(angle_full_[i]/4)
        window_out[i] = std::sin(angle_full_[i] * 0.25);
    }
}

void Spectrogram::windowHann()
{
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        window_out[i] = 0.5 * (1.0 - std::cos(angle_full_[i]));
    }
}

void Spectrogram::windowHamming()
{
    const double α = 25./46.;
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        window_out[i] = α * (1.0 - std::cos(angle_full_[i] * 0.5));
    }
}

void Spectrogram::windowBlackman()
{
    const double a0 = 0.42, a1 = 0.5, a2 = 0.08;
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        double θ = angle_full_[i] * 0.5;
        window_out[i] = a0 - a1*std::cos(θ) + a2*std::cos(2*θ);
    }
}

void Spectrogram::windowNuttall()
{
    const double a0=0.355768, a1=0.487396, a2=0.144232, a3=0.012604;
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        double θ = angle_full_[i] * 0.5;
        window_out[i] = a0 - a1*std::cos(θ) + a2*std::cos(2*θ) - a3*std::cos(3*θ);
    }
}

void Spectrogram::windowBlackmanNuttall()
{
    const double a0=0.3635819, a1=0.4891775, a2=0.1365995, a3=0.0106411;
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        double θ = angle_full_[i] * 0.5;
        window_out[i] = a0 - a1*std::cos(θ) + a2*std::cos(2*θ) - a3*std::cos(3*θ);
    }
}

void Spectrogram::windowBlackmanHarris()
{
    const double a0=0.35875, a1=0.48829, a2=0.14128, a3=0.01168;
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        double θ = angle_full_[i] * 0.5;
        window_out[i] = a0 - a1*std::cos(θ) + a2*std::cos(2*θ) - a3*std::cos(3*θ);
    }
}

void Spectrogram::windowFlatTop()
{
    const double a0=0.21557895,a1=0.41663158,a2=0.277263158,a3=0.083578947,a4=0.006947368;
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        double θ = angle_full_[i] * 0.5;
        window_out[i] = a0 - a1*std::cos(θ) + a2*std::cos(2*θ) - a3*std::cos(3*θ) + a4*std::cos(4*θ);
    }
}

void Spectrogram::windowBartlettHann()
{
    const double a0=0.62, a1=0.48, a2=0.38;
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        double t = std::abs(i/(double)frame_size_ - 0.5);
        window_out[i] = a0 - a1*t - a2*std::cos(angle_full_[i]*0.5);
    }
}

void Spectrogram::windowHannPoisson()
{
    const double a = 2.0;
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        double θ = angle_full_[i] * 0.5;
        double env = std::exp(-a * std::abs(0.5 - i/(double)frame_size_));
        window_out[i] = 0.5*(1.0 - std::cos(θ)) * env;
    }
}

void Spectrogram::addWindow()
{
    #pragma omp parallel for
    for(int i=0;i<frame_size_;++i){
        buffer_in_[i] = in[i] * window_out[i];
    }
}

void Spectrogram::fft()
{
    fftw_execute(plan_r2c_);

    #pragma omp parallel for
    for(int i=0;i<frame_size_/2;++i){
      fft_out[i] = buffer_out_[i];
    }
}

void Spectrogram::normalize(int multiplier)
{
    #pragma omp parallel for
    for(int i=0;i<frame_size_/2;++i){
        double db = multiplier * std::log10(std::norm(fft_out[i]) / frame_size_ + 1e-10f);
        if(db < db_range_.first)  db = db_range_.first + 1;
        if(db > db_range_.second) db = db_range_.second - 1;
        out[i] = db;
    }
}

double Spectrogram::interpolate(double f, double t, double p) {
    return f + (t - f) * p;
}

void Spectrogram::drawGraientLine(
  cv::Mat& img, int fill_type,
  const cv::Point& p0, const cv::Point& p1,
  const cv::Scalar& underline_color,
  int gradient_color
) {
  switch(fill_type)
  {
    case NOT_FILLED :
    {
      break;
    }

    case ONE_COLOR :
    {
      cv::line(img, p0, p1, underline_color, 1);
      break;
    }

    case GRADIENT :
    {
      cv::line(img, p0, p1, cv::Scalar(gradient_color, gradient_color, gradient_color), 1);
      break;
    }

    default :
    {
      cv::line(img, p0, p1, underline_color, 1);
      break;
    } 
  }
}

void a2i::Spectrogram::drawSpectrum(
  cv::Mat& img, 
  const int line_type, 
  const int graph_mode, 
  const int fill_type,
  const bool border_line,
  const cv::Scalar line_color, 
  const cv::Scalar underline_color,
  const int gradient_coefficient) 
{
  int y_first;
  double x_first = 0.0;
  bool first_point = true;

  std::vector<cv::Point> control_points;
  for(size_t i = 0; i < frame_size_ / 2; i += 1) 
  {
    double x;
    int y;
    double freq = i * static_cast<double>(sample_rate_) / frame_size_;
    
    switch(graph_mode)
    {
      case LIN :
      {
        x = std::min(static_cast<double>(img.cols), 
            std::max(0., (freq - freq_range_.first) * img.cols / (freq_range_.second - freq_range_.first)));
        break;
      }

      case LOG :
      {
        x = std::min(static_cast<double>(img.cols), 
            std::max(0., (std::log2(freq) - std::log2(freq_range_.first))  * img.cols / (std::log2(freq_range_.second) - std::log2(freq_range_.first))));
        break;
      }

      default :
      {
        x = std::min(static_cast<double>(img.cols), 
            std::max(0., (std::log2(freq) - std::log2(freq_range_.first))  * img.cols / (std::log2(freq_range_.second) - std::log2(freq_range_.first))));
        break;
      }
    }

    if(out[i] < 0) 
    {
      y = (1 - (std::abs(db_range_.first - out[i]) / std::abs(db_range_.second - db_range_.first))) * img.rows;
    }
    else 
    {
      y = (1 - ((std::abs(db_range_.first) + out[i]) / std::abs(db_range_.second - db_range_.first))) * img.rows;
    }

    if(freq >= freq_range_.first && freq <= freq_range_.second)
    {
      if(first_point)
      {
        control_points.push_back(cv::Point(x_first, y_first));
        first_point = false;
      }
      control_points.push_back(cv::Point(x, y));
      
    }
    else
    { 
      y_first = y;

      if(!first_point)
      {
        control_points.push_back(cv::Point(img.cols, y));
      }
    }
  }

  switch(line_type)
  {
    case BEZIE :
    {
      for(size_t i = 2; i < control_points.size() - 1; i+=3)
      {
        cv::Point p0 = control_points[i - 2];
        cv::Point p1 = control_points[i - 1];
        cv::Point p2 = control_points[i];
        cv::Point p3 = control_points[i + 1];

        double sstep = 0.6 / std::max(static_cast<double>(abs(p0.x - p3.x)), 0.0001);

        for(double j = 0; j < 1; j += sstep)
        {
          auto xa = interpolate(p0.x, p1.x ,j);
          auto ya = interpolate(p0.y, p1.y ,j);
          auto xb = interpolate(p1.x, p2.x ,j);
          auto yb = interpolate(p1.y, p2.y ,j);
          auto xc = interpolate(p2.x, p3.x ,j);
          auto yc = interpolate(p2.y, p3.y ,j);

          auto xm = interpolate(xa, xb, j);
          auto ym = interpolate(ya, yb, j);
          auto xn = interpolate(xb, xc, j);
          auto yn = interpolate(yb, yc, j);

          auto x = interpolate(xm, xn, j);
          auto y = interpolate(ym, yn, j);

          if(x >= img.cols) x = img.cols - 1;
          if(y >= img.rows) y = img.rows - 1;
          if(x <= 0) x = 1;
          if(y <= 0) y = 1;

          if(border_line)
          {
            img.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(line_color[0], line_color[1], line_color[2]);
          }

          int gradient_color = static_cast<int>((1 - y / static_cast<double>(img.rows)) * gradient_coefficient);
          drawGraientLine(img, fill_type, cv::Point(x, y), cv::Point(x, img.rows), underline_color, gradient_color);
        }
      }

      break;
    }

    case LINES :
    {
      for(size_t i = 0; i < control_points.size() - 1; i+=1)
      {
        cv::Point vertices[4];
        vertices[0] = control_points[i];
        vertices[1] = control_points[i+1];
        vertices[2] = cv::Point(control_points[i+1].x, img.rows);
        vertices[3] = cv::Point(control_points[i].x, img.rows);

        for (int x = vertices[0].x; x < vertices[1].x; x++)
        {
          double t = (double)(x - vertices[0].x) / (vertices[1].x - vertices[0].x);
          int y = interpolate(vertices[0].y, vertices[1].y, t);
          int gradient_color = static_cast<int>((1 - y / static_cast<double>(img.rows)) * gradient_coefficient);
          drawGraientLine(img, fill_type, cv::Point(x, y), cv::Point(x, img.rows), underline_color, gradient_color);
        }

        if(border_line)
        {
          cv::line(img, control_points[i], control_points[i+1], line_color, 1);
        }
        

      }

      break;
    }

    case BARS :
    {
      for(size_t i = 0; i < control_points.size(); i+=1) 
      {
        int gradient_color = static_cast<int>((1 - control_points[i].y / static_cast<double>(img.rows)) * gradient_coefficient);
        drawGraientLine(img, fill_type, cv::Point(control_points[i].x, control_points[i].y), cv::Point(control_points[i].x, img.rows), underline_color, gradient_color);
      }

      break;
    }
  }
}

void a2i::Spectrogram::drawGrid(
  cv::Mat& img, 
  int type, 
  bool enable_text, 
  const std::vector<unsigned int>& freqs, 
  int number_of_db_risks,
  cv::Scalar line_color, 
  cv::Scalar text_color) 
{
  std::vector<unsigned int> freq_risks;

  if(freqs.size() == 0)
  {
    freq_risks = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
  }
  else
  {
    for(const auto& freq : freqs)
    {      
      freq_risks.push_back(freq);
    }
  }

  int freq_size = freq_risks.size();

  float x = 0.0f;

  cv::line(img, cv::Point(x, 0), cv::Point(x, img.rows), line_color, 1);

  if(enable_text)
  {
    std::stringstream ss;
    ss << static_cast<int>(freq_risks[0]);
    cv::putText(img, ss.str(), cv::Point(x, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, text_color, 1);
  }

  for(int i = 1; i < freq_size; i++)
  {
    x = type ? 
      std::max(0., ((std::log2(freq_risks[i]) - std::log2(freq_risks[0])) / (std::log2(freq_range_.second) - std::log2(freq_risks[0]))) * img.cols)
    : std::max(0., static_cast<double>(freq_risks[i] - freq_risks[0])) * img.cols * 2 / sample_rate_;

    cv::line(img, cv::Point(x, 0), cv::Point(x, img.rows), line_color, 1);

    if(enable_text)
    {
      std::stringstream ss;
      ss << static_cast<int>(freq_risks[i]);
      cv::putText(img, ss.str(), cv::Point(x, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, text_color, 1);
    }
  }

  for(int i = 0; i < number_of_db_risks; i++)
  {
    float db = db_range_.first + i * (db_range_.second - db_range_.first) / number_of_db_risks;
    float y = (1 - (db - db_range_.first) / (db_range_.second - db_range_.first)) * img.rows;

    cv::line(img, cv::Point(0, y), cv::Point(img.cols, y), line_color, 1);

    if(enable_text)
    {
      std::stringstream ss;
      ss << static_cast<int>(db);
      cv::putText(img, ss.str(), cv::Point(15, y + 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, text_color, 1);
    }
  }
}