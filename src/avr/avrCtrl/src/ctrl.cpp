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

uint8_t buffer[13];

template<uint8_t n>
class RegisterAccessProtocol {

  struct Register {
    uint8_t len;
    enum Access {
      writeOnly,
      readOnly,
      both
    } access : 2;
    uint8_t* ptr;
  } registers[n];

  enum State {
    read,
    write,
    idle,
    cmd,
    retRead,
    retWrite
  } mState;

  enum Return {
    ok,
    aborted,
    invalidReg,
    invalidOp,
    invalidLen,
    invalidParam
  } mReturn;

  uint8_t mReg;
  uint8_t mBC;
  int8_t  mLen;
  union Command {
    struct {
      uint8_t reg  : 7;
      bool    write: 1;
    };
    uint8_t data;
    Command(uint8_t byte) : data(byte) {}
  };

  

  public:
  Delegate<uint8_t> transmitFunc;
    RegisterAccessProtocol() : mState(idle), mReturn(ok) {
      registers[0].len    = 1;
      registers[0].access = Register::readOnly;
      registers[0].ptr    = (uint8_t*)&mReturn;
      registers[1].len    =12;
      registers[1].access = Register::both;
      registers[1].ptr    = buffer;
    }

    void startFrame() {
      mState  = cmd;
      mLen = 0;
      mReturn = ok;
      mBC = 0;
    }

    void endFrame() {
      switch(mState) {
        case(cmd):      log::emit() << "SPI request terminated by master!" << " bc: " << (uint16_t)mBC << log::endl; mReturn = aborted; break;
        case(write):
        case(retWrite): log::emit() << "SPI write request aborted by master!" <<  " bc: " << (uint16_t)mBC <<log::endl; mReturn = aborted; break;
        case(read):
        case(retRead):  log::emit() << "SPI read  request aborted by master!" <<  " bc: " << (uint16_t)mBC <<log::endl; mReturn = aborted; break;
        case(idle):     if(mReturn == ok)
                          log::emit() << "SPI request completed!" << log::endl;
                        else
                          log::emit() << "SPI request failed: " << (uint16_t)mReturn << " reg: " << (uint16_t)mReg << " len: " << (uint16_t)mLen << " bc: " << (uint16_t)mBC << log::endl;
      }
      mState = idle;
      log::emit() << "Buffer: " << (char*)buffer << log::endl;
    }
    
    void handleRxByte(uint8_t byte) {
      mBC++;
      switch(mState) {
        case(cmd): {
          mReturn = ok;
          Command cmd(byte);
          mReg = cmd.reg;
          mState = cmd.write?retWrite:retRead;
          if(mReg>n || registers[mReg].len == 0) {
            mReturn = invalidReg;
            mState = idle;
          }
          if((registers[mReg].access== Register::writeOnly && !cmd.write) || (registers[mReg].access == Register::readOnly && cmd.write)) {
            mReturn = invalidOp;
            mState = idle;
          }
          transmitFunc(mReturn);
          break;
        }
        case(retWrite):
          mState = write;
        case(write):
          if(mLen<registers[mReg].len)
            registers[mReg].ptr[mLen]=byte;
          if(mLen++==registers[mReg].len-1)
            mState = idle;
          break;
        case(retRead):
          mState = read;
        case(read):
          if(mLen<registers[mReg].len)
            transmitFunc(registers[mReg].ptr[mLen]);
          if(mLen++==registers[mReg].len)
            mState = idle;
          break;
       default: if(mReturn == ok) mReturn = invalidLen;
     }
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
    
    void transmit(uint8_t byte) {
      UseRegMap(rm, Spi);
      SyncRegMap(rm);
      rm.spdr = byte;
    }

    SpiSlave() : Protocol() {
      
      UseRegMap(rm, Spi);
      rm.spie = true;
      rm.spe = true;
      rm.dord = false;
      rm.mstr = false;
      rm.cpol = false;
      rm.cpha = false;
      rm.miso.ddr = true;
      SyncRegMap(rm);

      this->setPullUp(true);
      this->FrameInt::enable();
      sInstance = this;
      this->transmitFunc.template bind<SpiSlave, &SpiSlave::transmit>(this);
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

typedef SpiSlave<RegisterAccessProtocol<2>, Spi, SpiFrameInt> Comm;

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
