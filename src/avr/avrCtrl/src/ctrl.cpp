#include <platform.h>

#include <avr-halib/drivers/avr/pwm.h>
#include <avr-halib/drivers/ext/led.h>
#include <avr-halib/interrupts/interrupt.h>
#include <avr-halib/interrupts/InterruptManager/SignalSemanticInterrupt.h>
#include <avr-halib/common/fixPoint.h>
#include <avr-halib/drivers/avr/externalInterrupt.h>
#include <avr-halib/common/delegate.h>

#include <boost/mpl/vector.hpp>

typedef avr_halib::object::FixPointValue<int32_t, 4> Angle;
typedef Angle Speed;


using avr_halib::regmaps::local::Timer1;
using avr_halib::regmaps::local::Timer2;
using avr_halib::config::PWMDefaultConfig;
using avr_halib::drivers::avr::PWMGenerator;
using avr_halib::drivers::ext::Led;

struct MotorConfig : public PWMDefaultConfig< Timer2 >
{
    enum ChannelUsage
    {
        useChannelA = true,
				useChannelB = false,
				useChannelC = false
    };

    static const pwm::Cycle      cycle      = pwm::static8;
    static const pwm::OutputMode outputMode = pwm::normal;
    static const pwm::Type       correction = pwm::phaseFreqCorrect;
    static const pwm::Prescalers ps         = pwm::ps256;
};

struct ServoConfig : public PWMDefaultConfig< Timer1 >
{
    enum ChannelUsage
    {
        useChannelA = true,
				useChannelB = false,
				useChannelC = false
    };

    static const pwm::Cycle      cycle      = pwm::static10;
    static const pwm::OutputMode outputMode = pwm::normal;
    static const pwm::Type       correction = pwm::fast;
    static const pwm::Prescalers ps         = pwm::ps256;
};

typedef PWMGenerator< ServoConfig > Servo;
typedef PWMGenerator< MotorConfig > Motor;

Servo servo;
Motor motor;

/** \brief change steering angle of car
 *  \param angle steering angle in milliDeg (left is negative)
 */
void steer(Angle angle) {
  const Angle ticksPerUS(1000000ULL*Servo::TickFrequency::denominator/Servo::TickFrequency::numerator);
  const Angle maxAngle(30);
  const Angle maxUS(2000);
  const Angle straightUS(1500);
  const Angle calibFactor(-1);
  const Angle calibOffset(0);
  angle = angle * calibFactor + calibOffset;
  angle = angle * (maxUS-straightUS) / maxAngle;
  angle = (angle + straightUS) / ticksPerUS;
  log::emit() << "Steering angle: " << angle << log::endl;
  servo.value<Servo::channelA>(angle.trunc());
}

/** \brief change steering angle of car
 *  \param angle steering angle in milliDeg (left is negative)
 */
void speed(Speed speed) {
  const Speed maxSpeed(14);
  UsePortmap(dir, platform::Motor);
    dir.dir.port = speed>0;
  SyncPortmap(dir);
  speed = speed.abs() * Motor::pwmMax / maxSpeed;
  log::emit() << "Speed: " << speed << log::endl;
  motor.value<Motor::channelA>(speed.trunc());
}

using avr_halib::regmaps::local::Spi;
using avr_halib::common::Delegate;


class Protocol {
  enum State {
    idle,
    write,
    read
  } mState;
  enum Return {
    ok,
    invalidReg,
    invalidOp,
    invalidParam
  } mReturn;
  uint8_t mRxBuffer[129];
  uint8_t mTxBuffer[129];
  uint8_t mRxPtr;
  uint8_t mTxPtr;

  Delegate<uint8_t> transmitFunc;

  public:
    Protocol() : mState(idle), mReturn(ok), mRxPtr(0), mTxPtr(0) {}

    void startFrame() {
      mState  = idle;
      mRxPtr  =    0;
      mTxPtr  =    0;
      mReturn =   ok;
    }

    void endFrame() {
      mRxBuffer[mRxPtr] = '\0';
      log::emit() << "Got SPI Packet: " << (char*)mRxBuffer << log::endl;
    }
    
    void handleRxByte(uint8_t byte) {
      mRxBuffer[mRxPtr++]=byte;
      mRxPtr%=(sizeof(mRxBuffer)-1);
    }
};


using namespace avr_halib::interrupts::interrupt_manager;
using boost::mpl::vector;

using avr_halib::drivers::avr::ExternalInterrupt;

template<typename Proto, typename SpiRegMap, typename ExtIntRegMap> 
class SpiSlave : public Proto,
                 private ExternalInterrupt<ExtIntRegMap, ExtIntRegMap::both> {
  private:
    typedef Proto Protocol;
    typedef ExternalInterrupt<ExtIntRegMap, ExtIntRegMap::both> FrameInt;
  public:

    static SpiSlave* sInstance;
    
    SpiSlave() : Protocol() {
      
      UseRegMap(rm, Spi);
      rm.spie = true;
      rm.spe = true;
      rm.dord = false;
      rm.mstr = false;
      rm.cpol = false;
      rm.cpha = false;
      SyncRegMap(rm);

      this->setPullUp(true);
      this->FrameInt::enable();
      sInstance = this;
    }

    void handleOp() {
      UseRegMap(rm, Spi);
      this->handleRxByte(rm.spdr);
    }

    void handleFrame() {
      UseRegMap(rm, Spi);
      SyncRegMap(rm);
      if(rm.ss.pin)
        this->endFrame();
      else
        this->startFrame();
}
  private:
    typedef Slot<Spi::InterruptMap::operationComplete, Binding::SingletonFunction> SpiSlot;
    typedef Slot<FrameInt::InterruptMap::externalInterrupt, Binding::SingletonFunction> SpiFrameSlot;
    typedef typename SpiSlot::template Bind<SpiSlave, &SpiSlave::handleOp, &sInstance> BoundSpiSlot;
    typedef typename SpiFrameSlot::template Bind<SpiSlave, &SpiSlave::handleFrame, &sInstance> BoundSpiFrameSlot;
  public:
    typedef typename vector<BoundSpiSlot, BoundSpiFrameSlot>::type InterruptSlotList;
};

template<typename P, typename I, typename R>
SpiSlave<P, I, R>* SpiSlave<P, I, R>::sInstance = NULL;

typedef avr_halib::regmaps::local::ExternalInterrupt6 SpiFrameInt;

typedef SpiSlave<Protocol, Spi, SpiFrameInt> Comm;

typedef InterruptManager< Comm::InterruptSlotList > IM;

BIND_INTERRUPTS(IM);

int main()
{

    log::emit() << "Servo Tick-Size: " << (1000000ULL * Servo::TickFrequency::denominator / Servo::TickFrequency::numerator) << "  us" << log::endl;
    log::emit() << "Servo PWM-Freq : " << (Servo::DutyFrequency::numerator / Servo::DutyFrequency::denominator) << "  Hz" << log::endl;
    log::emit() << "Motor Tick-Size: " << (1000000ULL * Motor::TickFrequency::denominator / Motor::TickFrequency::numerator) << "  us" << log::endl;
    log::emit() << "Motor PWM-Freq : " << (Motor::DutyFrequency::numerator / Motor::DutyFrequency::denominator) << "  Hz" << log::endl;

    Comm comm;

    motor.start();
		servo.start();

    UsePortmap(motorPins, platform::Motor);
    motorPins.dir.ddr  = true;
    motorPins.dir.port = platform::Motor::onLevel;
    motorPins.reset.ddr = true;
    motorPins.reset.port = true;
    motorPins.ff1.port = true;
    motorPins.ff2.port = true;
    SyncPortmap();

    Led<platform::StatusLed> status;
    status.setOn();

    steer(-10.0f);
    speed(1.1f);

    sei();

    while(true)
        Morpheus::sleep(Morpheus::SleepModes::idle);

    return 0;
}
