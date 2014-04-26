#include <Motor.h>
#include <MotorError.h>

#include <ostream>

#include <error.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>

#include <bcm2835.h>

using namespace std;
using boost::errinfo_errno;
using boost::throw_function;
using boost::throw_file;
using boost::throw_line;


static timespec operator+(timespec time, unsigned long long duration){
  timespec temp=time;
  temp.tv_nsec+=duration;
  temp.tv_sec+=temp.tv_nsec/1000000000ULL;
  temp.tv_nsec%=1000000000ULL;
  return temp;
}

ostream& operator<<(ostream& out, const timespec& time){
  return out << time.tv_sec << "." << time.tv_nsec << "s";
}

void Motor::motorTask( Motor& motor){
  timespec now;
  try{
    if(mlockall(MCL_CURRENT | MCL_FUTURE))
      throw MotorError::RTError() << errinfo_errno(errno)
                                  << throw_function(__PRETTY_FUNCTION__)
                                  << throw_file(__FILE__)
                                  << throw_line(__LINE__);
  
    sched_param sParam;
    sParam.sched_priority=99;
    if(pthread_setschedparam(motor.mThread.native_handle(), SCHED_FIFO, &sParam))
      throw MotorError::RTError() << errinfo_errno(errno)
                                  << throw_function(__PRETTY_FUNCTION__)
                                  << throw_file(__FILE__)
                                  << throw_line(__LINE__);

    bcm2835_init();
    bcm2835_gpio_fsel(motor.mConfig.pin, BCM2835_GPIO_FSEL_OUTP);
    if(clock_gettime(CLOCK_MONOTONIC, &now))
      throw MotorError::ClockError() << errinfo_errno(errno)
                                     << throw_function(__PRETTY_FUNCTION__)
                                     << throw_file(__FILE__)
                                     << throw_line(__LINE__);
  }
  catch(std::exception& e){
    motor.errorCallback(e);
    return;
  }
  unsigned long long maxTicks = 1ULL << motor.mConfig.bits;
  unsigned long long baseTick = 1000000000ULL / motor.mConfig.frequency  / maxTicks;
  while(!motor.stopped())
  {
    try{
      bcm2835_gpio_set(motor.mConfig.pin);
      now=now + (baseTick * motor.mSpeed.load());
      if(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &now, NULL))
        throw MotorError::SleepError() << errinfo_errno(errno)
                                       << throw_function(__PRETTY_FUNCTION__)
                                       << throw_file(__FILE__)
                                       << throw_line(__LINE__);

      bcm2835_gpio_clr(motor.mConfig.pin);
      now=now + (baseTick * (maxTicks-motor.mSpeed.load()));
      if(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &now, NULL))
        throw MotorError::SleepError() << errinfo_errno(errno)
                                       << throw_function(__PRETTY_FUNCTION__)
                                       << throw_file(__FILE__)
                                       << throw_line(__LINE__);
    }catch(std::exception& e){
      motor.errorCallback(e);
    }
  }
}

Motor::Motor(const Motor::Config& config) : 
  mConfig(config), mThread(&Motor::motorTask, ref(*this)), mStop(false){
    speed(0);
}

void Motor::speed(Motor::SpeedType value)
{
  if(value > mConfig.max || value < mConfig.min){
    throw MotorError::InvalidSpeed() << MotorError::SpeedInfo(value)
                                     << throw_function(__PRETTY_FUNCTION__)
                                     << throw_file(__FILE__)
                                     << throw_line(__LINE__);
  }
  mSpeed=value*mConfig.m/1000+mConfig.n;
}

Motor::SpeedType Motor::speed() const{
  return (mSpeed.load()-mConfig.n)/mConfig.m;
}

Motor::~Motor(){
  mStop=true;
  mThread.join();
}

std::ostream& operator<<(std::ostream& out, const Motor& m){
  if(!m.stopped())
    return out << "Motor: " << m.speed();
  else
    return out << "Motor: stopped";
}

std::ostream& operator<<(std::ostream& out, const Motor::Config& cfg){
  return out << "Motor(GPIO " << (uint16_t)cfg.pin << "): v = " << cfg.m << " * x / 1000 + " << cfg.n << " - " << cfg.min << "m/s < v < " << cfg.max << "m/s - " << "PWM: " << cfg.frequency << "Hz, " << (uint16_t)cfg.bits << "bit";
}

void Motor::addErrorHandler(ErrorHandlerType handler){
  errorCallback.connect(handler);
}
