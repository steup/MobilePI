#pragma once

#include <sstream>
#include <exception>
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/get_error_info.hpp>

class MotorError : public virtual boost::exception,
                   public virtual std::exception{
  public:
    enum Cause{
      InvalidSpeed
      RTError,
      ClockError,
      SleepError
    };
    using SpeedInfo = boost::error_info< struct SpeedInfoTag, int >;
  private:
    Cause mCode;
    MotorError(Cause cause) : mCause(cause){}
  public:
    static MotorError InvalidSpeed(){return MotorError(Cause::InvalidSpeed);}
    virtual const char* what() const throw(){
      const boost::throw_function::value_type* const funcPtr  = boost::get_error_info<boost::throw_function>(*this);
      const boost::throw_file::value_type* const     filePtr  = boost::get_error_info<boost::throw_file>(*this);
      const boost::throw_line::value_type* const     linePtr  = boost::get_error_info<boost::throw_line>(*this);
      const SpeedInfo::value_type* const             speedPtr = boost::get_error_info<SpeedInfo>(*this);
      const errinfo_errno::value_type* const         errnoPtr = boost::get_error_info<errinfo_errno>(*this);

      std::stringstream msg;
      if(funcPtr && filePtr && linePtr)
        msg << *funcPtr << "[" << *filePtr << ":" << *linePtr << "] ";
      msg << "MotorError - ";
      switch(mCause){
        case(Cause::InvalidSpeed): msg << "speed not valid: ";
                                   break;
        case(Cause::RTError)     : msg << "real-time scheduling not available: ";
                                   break;
        case(Cause::ClockError)  : msg << "monotonic real-time clock not available: ";
                                   break;
        case(Cause::SleepError)  : msg << "real-time sleep error: ";
                                   break;
        default                  : msg << "unknown";
                                   break;
      }
      if(speedPtr)
        msg << *speedPtr;
      if(errnoPtr)
        msg << *errnoPtr;
      return msg.str().c_str();
    }
    Cause cause() const{return mCause;}
    bool operator==(const MotorError& e) const{return mCause==e.mCause;}
};
