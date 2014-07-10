#pragma once

#include <exception>
#include <string>
#include <sstream>

class Exception : public std::exception{
  private:
  std::string msg;

  public:
    template<typename T>
    Exception& operator<<(const T& value){
      std::ostringstream o;
      o << value;
      msg += o.str();
      return *this;
    }

    Exception& operator<<(std::ostream& (*f)(std::ostream&)){
      std::ostringstream o;
      o << f;
      msg += o.str();
      return *this;
    }
    
    virtual const char* what() const throw(){ return msg.c_str();}
};
