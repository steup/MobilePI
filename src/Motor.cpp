#include <Motor.h>

#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>

#include <bcm2835.h>

using namespace std;

MotorException::MotorException(Cause code) throw() : mCode(code){}

timespec operator+(timespec time, unsigned long long duration){
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
  mlockall(MCL_CURRENT | MCL_FUTURE);
  sched_param sParam;
  timespec now;
  timespec resolution;
  int retval;
  
  sParam.sched_priority=99;
  if(pthread_setschedparam(motor.mThread.native_handle(), SCHED_FIFO, &sParam))
    cerr << "Error: creating rt-thread: " << strerror(errno) << endl;
  unsigned long long maxTicks = 1ULL << motor.mConfig.bits;
  unsigned long long baseTick = 1000000000ULL / motor.mConfig.frequency  / maxTicks;
  clock_getres(CLOCK_MONOTONIC, &resolution);
  cout << "Resolution: " << resolution << endl;
  bcm2835_init();
  bcm2835_gpio_fsel(motor.mConfig.pin, BCM2835_GPIO_FSEL_OUTP);
  retval=clock_gettime(CLOCK_MONOTONIC, &now);
  if(retval)
    cerr << "Error: getting current time of CLOCK_REALTIME: " << strerror(retval) << endl;
  while(!motor.stopped())
  {
    bcm2835_gpio_set(motor.mConfig.pin);
    now=now + baseTick * motor.mSpeed.load();
    retval = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &now, NULL);
    if(retval)
      cerr << "Error: sleeping for " << now << ": " << strerror(retval) << endl;
    bcm2835_gpio_clr(motor.mConfig.pin);
    now=now + baseTick * (maxTicks-motor.mSpeed.load());
    retval = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &now, NULL);
    if(retval)
      cerr << "Error: sleeping for " << now << ": " << strerror(retval) << endl;
  }
}

const char* MotorException::what() const throw()
{
  string msg="Error ";
  switch(mCode)
  {
    case(invalidSpeed): msg+="speed exceeding max. speed";
                        break;
    case(noModule)    : msg+="swpwm module not loaded";
  }
  return msg.c_str();
}

Motor::Motor(const MotorConfig& config) : 
  mConfig(config), mThread(&Motor::motorTask, ref(*this)), mStop(false){
    stringstream s;
    s << "GPIO " << mConfig.pin << " PWM";
    speed(0);
}

void Motor::speed(Motor::SpeedType value)
{
  if(value > mConfig.max){
    mSpeed=mConfig.max*mConfig.m+mConfig.n;
    throw MotorException(MotorException::invalidSpeed);
  }
  if(value < mConfig.min){
    mSpeed=mConfig.min*mConfig.m+mConfig.n;
    throw MotorException(MotorException::invalidSpeed);
  }
  mSpeed=(uint32_t)value*mConfig.m/1000+mConfig.n;
}

Motor::SpeedType Motor::speed() const{
  return (mSpeed.load()-mConfig.n)/mConfig.m;
}

Motor::~Motor(){
  mStop=true;
  mThread.join();
}
