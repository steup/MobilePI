#include <Servo.h>
#include <ServoError.h>

#include <bcm2835.h>

using namespace boost;

static const unsigned long PWMBaseClock=19200000UL;

Servo::Servo(const Config& config) : mConfig(config){
	if(!bcm2835_init())
    throw ServoError::InitError() << throw_function(__PRETTY_FUNCTION__)
                                  << throw_file(__FILE__)
                                  << throw_line(__LINE__);

	bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_ALT5);
	bcm2835_pwm_set_clock( (PWMBaseClock >> mConfig.bits) / mConfig.frequency );
	bcm2835_pwm_set_mode(0, 1, 1);
	bcm2835_pwm_set_range(0, 1<<mConfig.bits);
  angle(0);
}

void Servo::angle(int angle)
{
  if(angle<mConfig.min || angle>mConfig.max)
    throw ServoError::InvalidAngle() << ServoError::AngleInfo(angle)
                                     << throw_function(__PRETTY_FUNCTION__)
                                     << throw_file(__FILE__)
                                     << throw_line(__LINE__);
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
  return out << "Servo(GPIO 18): theta= " << cfg.m << " * x / 1000 + " << cfg.n 
             << ": " << cfg.min << "deg <= theta <= " << cfg.max << "deg - "
             << "PWM: " << (PWMBaseClock >> cfg.bits)/cfg.frequency 
             << "Hz, " << (uint16_t)cfg.bits << "bit";
}
