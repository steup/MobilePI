#pragma once

#include <thread>
#include <atomic>
#include <ostream>

#include <boost/signals2/signal.hpp>

class Motor{
  public:
    typedef int SpeedType;
    struct Config{
      unsigned int  n         = 100;
      int  m                  = 100;
      SpeedType     max       = 100;
      SpeedType     min       = -50;
      unsigned int  frequency =  75;
      unsigned char pin       =  23;
      unsigned char bits      =  10;
    };

  private:
    std::atomic<SpeedType> mSpeed;
    const Config      mConfig;
    std::thread            mThread;
    bool                   mStop;
    boost::signals2::signal<void (std::exception& e)> errorCallback;
    static void motorTask(Motor& motor);
  public:
    using ErrorHandlerType = boost::signals2::signal<void (std::exception&)>::slot_type;

    Motor(const Config& config);
    ~Motor();
    void speed(SpeedType speed);
    SpeedType speed() const;
    const Config& config() const {return mConfig;}
    bool stopped() const {return mStop;}
    void addErrorHandler(ErrorHandlerType handler);
};

std::ostream& operator<<(std::ostream& out, const Motor& s);
std::ostream& operator<<(std::ostream& out, const Motor::Config& c);
