#pragma once

#include <sstream>
#include <exception>
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/get_error_info.hpp>

class ServoError : public virtual boost::exception,
                   public virtual std::exception{
  public:
    enum class Cause{
      InvalidAngle,
      InitError
    };
    using AngleInfo = boost::error_info< struct AngleInfoTag, int >;
  private:
    Cause mCause;
    ServoError(Cause cause) : mCause(cause){}
  public:
    static ServoError InvalidAngle() {return ServoError(Cause::InvalidAngle);}
    static ServoError InitError  () {return ServoError(Cause::InitError);  }

    virtual const char* what() const throw(){
      const boost::throw_function::value_type* const funcPtr  = boost::get_error_info<boost::throw_function>(*this);
      const boost::throw_file::value_type* const     filePtr  = boost::get_error_info<boost::throw_file>(*this);
      const boost::throw_line::value_type* const     linePtr  = boost::get_error_info<boost::throw_line>(*this);
      const AngleInfo::value_type* const             anglePtr = boost::get_error_info<AngleInfo>(*this);

      std::stringstream msg;
      if(funcPtr && filePtr && linePtr)
        msg << *funcPtr << "[" << *filePtr << ":" << *linePtr << "] ";
      msg << "ServoError - ";
      switch(mCause){
        case(Cause::InvalidAngle): msg << "angle not valid: ";
                                   break;
        case(Cause::InitError)   : msg << "error initializing bcm2835 access";
                                   break;
        default                  : msg << "unknown";
                                   break;
      }
      if(anglePtr)
        msg << *anglePtr;
      return msg.str().c_str();
    }
    Cause cause() const{return mCause;}
    bool operator==(const ServoError& e) const{return mCause==e.mCause;}
};
