#include <Servo.h>

#include <string>
#include <cstring>

#include <bcm2835.h>

static const unsigned long PWMBaseClock=19200000UL;

ServoException::ServoException(Cause code) throw() : mError(errno), mCode(code){}

const char* ServoException::what() const throw()
{
  std::string msg="Error ";
  switch(mCode)
  {
    case(initError)   : msg+="initializing interface to bcm2835: ";
                        msg+=strerror(mError);
                        break;
    case(invalidAngle): msg+="angle exceeding max. angle";
                        break;
  }
  return msg.c_str();
}

Servo::Servo(const Config& config) : mConfig(config){
	if(!bcm2835_init())
    throw ServoException(ServoException::initError);
	bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_ALT5);
	//bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_256);
	bcm2835_pwm_set_clock( (PWMBaseClock >> mConfig.bits) / mConfig.frequency );
	bcm2835_pwm_set_mode(0, 1, 1);
	bcm2835_pwm_set_range(0, 1<<mConfig.bits);
  angle(0);
}

void Servo::angle(int angle)
{
  if(angle<mConfig.min || angle>mConfig.max)
    throw ServoException(ServoException::invalidAngle);
  mAngle=angle;
	bcm2835_pwm_set_data(0, mAngle*mConfig.m/1000 + mConfig.n);
}

Servo::~Servo(){
	angle(0);
  bcm2835_close();
}


std::ostream& operator<<(std::ostream& out, const Servo& s){
  return out << "Servo(GPIO 18): theta = " << s.angle();
}

std::ostream& operator<<(std::ostream& out, const Servo::Config& cfg){
  return out << "Servo(GPIO 18): theta= " << cfg.m << " * x / 1000 + " << cfg.n << ": " 
             << cfg.min << "deg <= theta <= " << cfg.max << "deg - "
             << "PWM: " << (PWMBaseClock >> cfg.bits)/cfg.frequency << "Hz, " << (uint16_t)cfg.bits << "bit";
}
