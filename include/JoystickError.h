#pragma once

#include <exception>

#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>

class JoystickError : public virtual boost::exception,
                      public virtual std::exception {
  public:
    enum class Cause{
      NoSuchJoystick,
      InitError,
      EventError
    };
    using NumberInfo = boost::error_info< struct NumberInfoTag, int         >;
    using SDLInfo    = boost::error_info< struct SDLInfoTag   , const char* >;
  private:
    Cause mCause;
    JoystickError(Cause cause) throw() : mCause(cause){}
  public:
    static JoystickError NoSuchJoystick() { return JoystickError( Cause::NoSuchJoystick ); }
    static JoystickError InitError()      { return JoystickError( Cause::InitError ); }
    static JoystickError EventError()     { return JoystickError( Cause::EventError ); }
    Cause cause() const                   { return mCause; }
    bool operator==( const JoystickError& a ) const { return mCause == a.mCause; }
    virtual const char* what() const throw();
};
