#pragma once
#include <exception>

#include <ostream>

class ServoException : public std::exception{
  public:
    enum Cause{
      initError,
      invalidAngle
    };
  private:
    const int mError;
    Cause mCode;
  public:
    ServoException(Cause code) throw();
    virtual const char* what() const throw();
    int error() const{return mError;}
    Cause code() const{return mCode;}
    bool operator==(const ServoException& e) const{return mCode==e.mCode;}
};

class Servo{
  private:
    int mAngle;
    const unsigned int mOffset;
    const int mFactor;
    const unsigned int mMaxAngle;
  public:
    Servo(unsigned int maxAngle=250, unsigned int offset=1065, int factor=1);
    ~Servo();
    void angle(int angle);
    int angle() const{return mAngle;}
    unsigned int offset() const{return mOffset;}
    unsigned int factor() const{return mFactor;}
    unsigned int maxAngle() const{return mMaxAngle;}
};

std::ostream& operator<<(std::ostream& out, const Servo& s);
