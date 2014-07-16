#pragma once

#include <exception>
#include <sstream>

#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/get_error_info.hpp>

class JoystickError : public virtual boost::exception,
                      public virtual std::exception{
  public:
    enum class Cause{
      NoSuchJoystick,
      InitError,
      EventError
    };
    using NumberInfo = boost::error_info< struct NumberInfoTag, int   >;
    using SDLInfo    = boost::error_info< struct SDLInfoTag,    const char* >;
  private:
    Cause mCause;
    JoystickError(Cause cause) throw() : mCause(cause){}
  public:
    static JoystickError NoSuchJoystick(){return JoystickError(Cause::NoSuchJoystick);}
    static JoystickError InitError(){return JoystickError(Cause::InitError);}
    static JoystickError EventError(){return JoystickError(Cause::EventError);}
    virtual const char* what() const throw(){
      const boost::throw_function::value_type* const funcPtr  = boost::get_error_info<boost::throw_function>(*this);
      const boost::throw_file::value_type* const     filePtr  = boost::get_error_info<boost::throw_file>(*this);
      const boost::throw_line::value_type* const     linePtr  = boost::get_error_info<boost::throw_line>(*this);
      const NumberInfo::value_type* const            numPtr   = boost::get_error_info<NumberInfo>(*this);
      const SDLInfo::value_type* const               errorPtr = boost::get_error_info<SDLInfo>(*this);

      std::stringstream msg;
      if(funcPtr && filePtr && linePtr)
        msg << *funcPtr << "[" << *filePtr << ":" << *linePtr << "] ";
      msg << "JoystickError - ";
      switch(mCause){
        case(Cause::NoSuchJoystick): msg << "No such joystick ";
                                     break;
        case(Cause::InitError)     : msg << "Error initalizing joystick ";
                                     break;
        case(Cause::EventError)    : msg << "Error waiting for next joystick event ";
                                     break;
        default                    : msg << "unknown";
                                     break;
      }
      if(numPtr)
        msg << *numPtr;
      if(errorPtr)
        msg << *errorPtr;
      return msg.str().c_str();
    }
    Cause cause() const{return mCause;}
    bool operator==(const JoystickError& a) const{return mCause==a.mCause;}
};
