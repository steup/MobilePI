#include <Servo.h>

#include <string>
#include <cstring>

#include <bcm2835.h>

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

Servo::Servo(unsigned int maxAngle, unsigned int offset, unsigned int factor) : mOffset(offset), mFactor(factor), mMaxAngle(maxAngle){
	if(!bcm2835_init())
    throw ServoException(ServoException::initError);
	bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_ALT5);
	//bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_256);
	bcm2835_pwm_set_clock(27);
	bcm2835_pwm_set_mode(0, 1, 1);
	bcm2835_pwm_set_range(0, 10000);
  angle(0);
}

void Servo::angle(int angle)
{
  if(abs(angle)>mMaxAngle)
    throw ServoException(ServoException::invalidAngle);
  mAngle=angle;
	bcm2835_pwm_set_data(0, mAngle*mFactor+mOffset);
}

Servo::~Servo(){
	angle(0);
  bcm2835_close();
}


std::ostream& operator<<(std::ostream& out, const Servo& s){
  return out << "Servo(GPIO 18): y = " << s.config().m << " * x + " << s.config().n << " < " << s.config().max;
}
