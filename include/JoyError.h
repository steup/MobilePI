#pragma once

#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <exception>

class JoyError : public virtual boost::exception,
                 public virtual std::exception{
  public:
    enum class Cause{
      NoSuchJoystick,
      InitError,
      EventError
    };
    using NumberInfo = boost::error_info< struct NumberInfo, int   >;
    using SDLInfo    = boost::error_info< struct SDLInfo,    char* >;
  private:
    Cause mCause;
    JoyError(Cause cause) throw() : mCause(cause){}
  public:
    static JoyError NoSuchJoystick(){return JoyError(Cause::NoSuchJoystick);}
    static JoyError InitError(){return JoyError(Cause::InitError);}
    static JoyError EventError(){return JoyError(Cause::EventError);}
    virtual const char* what() const throw(){
      switch(mCause){
        case(Cause::NoSuchJoystick): return "JoystickError: No such joystick";
        case(Cause::InitError)     : return "JoystickError: error initalizing joystick";
        case(Cause::EventError)    : return "JoystickError: error waiting for next joystick event";
        default             : return "JoystickError: unknown";
      }
    }
    bool operator==(const JoyError& a) const{return mCause==a.mCause;}
};
