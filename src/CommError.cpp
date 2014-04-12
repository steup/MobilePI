#include <CommError.h>

#include <string>

using namespace std;

CommError::CommError(Cause code) throw() : mCode(code), mError(errno){}

const char* CommError::what() const throw(){
  string msg="Error: ";
  switch(mCode)
  {
    case(ledError)        : msg+="led does not exist";
                            break;
    case(brightnessError) : msg+="brightness not supported";
                            break;
    case(speedError)      : msg+="speed not supported";
                            break;
    case(angleError)      : msg+="steering angle not supported";
                            break;
    case(invalidData)     : msg+="received data not valid";
                            break;
    case(timeout)         : msg+="receive timed out";
                            break;
  }
  return msg.c_str();
}
