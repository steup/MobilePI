#pragma once
#include <exception>
#include <thread>
#include <atomic>

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
    struct MotorConfig{
      MotorConfig(){}
      unsigned int  n         = 100;
      unsigned int  m         = 100;
      SpeedType     max       = 100;
      SpeedType     min       = -50;
      unsigned int  frequency =  75;
      unsigned char pin       =  23;
      unsigned char bits      =  10;
    };

  private:
    std::atomic<SpeedType> mSpeed;
    const MotorConfig      mConfig;
    std::thread            mThread;
    bool                   mStop;
    static void motorTask(Motor& motor);
  public:
    Motor(const MotorConfig& config=MotorConfig());
    ~Motor();
    void speed(SpeedType speed);
    SpeedType speed() const;
    const MotorConfig& config() const {return mConfig;}
    bool stopped() const {return mStop;}
};
