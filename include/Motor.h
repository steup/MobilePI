#pragma once
#include <exception>
#include <thread>
#include <atomic>
#include <ostream>

class MotorException : public std::exception{
  public:
    enum Cause{
      invalidSpeed,
      noModule
    };
  private:
    Cause mCode;
  public:
    MotorException(Cause code) throw();
    virtual const char* what() const throw();
    Cause code() const{return mCode;}
    bool operator==(const MotorException& e) const{return mCode==e.mCode;}
};

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
    static void motorTask(Motor& motor);
  public:
    Motor(const Config& config);
    ~Motor();
    void speed(SpeedType speed);
    SpeedType speed() const;
    const Config& config() const {return mConfig;}
    bool stopped() const {return mStop;}
};

std::ostream& operator<<(std::ostream& out, const Motor& s);
std::ostream& operator<<(std::ostream& out, const Motor::Config& c);
