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
  public:
    struct Config{
      int min=-100;
      int max=100;
      unsigned int n=100;
      int m=1;
      unsigned int frequency = 50;
      uint8_t bits = 10;
    };

  private:
    int mAngle;
    const Config mConfig;
  public:
    Servo(const Config& config);
    ~Servo();
    void angle(int angle);
    int angle() const{return mAngle;}
    const Config& config() const{return mConfig;}
};

std::ostream& operator<<(std::ostream& out, const Servo& s);
std::ostream& operator<<(std::ostream& out, const Servo::Config& cfg);
