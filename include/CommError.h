#pragma once

#include <sstream>
#include <exception>
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/errinfo_errno.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/system/error_code.hpp>

class CommError : public virtual boost::exception,
                   public virtual std::exception{
  public:
    enum class Cause{
      IOError,
      InvalidData,
      LedError,
      Timeout
    };
    using LedInfo = boost::error_info< struct LedInfoTag, uint8_t >;
    using IOErrorInfo = boost::error_info< struct IOErrorInfoTag, boost::system::error_code>;
  private:
    Cause mCause;
    CommError(Cause cause) : mCause(cause){}
  public:
    static CommError IOError    () {return CommError(Cause::IOError);    }
    static CommError InvalidData() {return CommError(Cause::InvalidData);}
    static CommError LedError   () {return CommError(Cause::LedError);   }
    static CommError Timeout    () {return CommError(Cause::Timeout);    }

    virtual const char* what() const throw(){
      const boost::throw_function::value_type* const funcPtr  = boost::get_error_info<boost::throw_function>(*this);
      const boost::throw_file::value_type* const     filePtr  = boost::get_error_info<boost::throw_file>(*this);
      const boost::throw_line::value_type* const     linePtr  = boost::get_error_info<boost::throw_line>(*this);
      const LedInfo::value_type* const               ledPtr   = boost::get_error_info<LedInfo>(*this);
      const IOErrorInfo::value_type* const           ioPtr    = boost::get_error_info<IOErrorInfo>(*this);
      std::stringstream msg;
      if(funcPtr && filePtr && linePtr)
        msg << *funcPtr << "[" << *filePtr << ":" << *linePtr << "] ";
      msg << "CommError - ";
      switch(mCause){
        case(Cause::IOError)         : msg << "io error: ";
                                       break;
        case(Cause::LedError)        : msg << "led does not exist: ";
                                       break;
        case(Cause::InvalidData)     : msg << "received data is invalid";
                                       break;
        case(Cause::Timeout)         : msg << "no data received in time";
                                       break;
        default                      : msg << "unknown";
                                       break;
      }
      if(ledPtr)
        msg << *ledPtr;
      if(ioPtr)
        msg << ioPtr->message();
      return msg.str().c_str();
    }
    Cause cause() const{return mCause;}
    bool operator==(const CommError& e) const{return mCause==e.mCause;}
};
