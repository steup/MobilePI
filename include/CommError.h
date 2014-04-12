#pragma once
#include <exception>

class CommError : public std::exception{
  public:
    enum Cause{
      ledError,
      brightnessError,
      speedError,
      angleError,
      invalidData,
      timeout
    };
  private:
    Cause mCode;
    int mError;
  public:
    CommError(Cause code) throw();
    virtual const char* what() const throw();
    const Cause& code() const throw() {return mCode;}
    bool operator==(const CommError& e) const throw() {return mCode==e.mCode;}
};
