#pragma once
#include <cstdint>
#include <ostream>

namespace CommData{
  struct InitData{
    InitData(int32_t maxSpeed=100, int32_t maxAngle=300, uint8_t maxBrightness=255, uint8_t numLeds=4) :
      maxSpeed(maxSpeed), maxAngle(maxAngle), maxBrightness(maxBrightness), numLeds(numLeds){}
    int32_t maxSpeed, maxAngle;
    uint8_t maxBrightness, numLeds;
  } __attribute__((packed));
  struct MoveData{
    MoveData(int32_t speed=0, int32_t angle=0) 
      : speed(speed), angle(angle){}
    bool operator==(const MoveData& a){return speed==a.speed && angle==a.angle;}
    int32_t speed, angle;
  } __attribute__((packed));
  struct LedData{
    LedData(uint8_t red=0, uint8_t green=0, uint8_t blue=0) : 
      red(red), green(green), blue(blue) {}
    uint8_t red, green, blue;
    bool operator==(const LedData& a){return red==a.red && green==a.green && blue==a.blue;}
  } __attribute__((packed));
}

inline std::ostream& operator<<(std::ostream& out, const CommData::InitData& data){
  return out << "(max. Speed, max. Angle, max. Brightness, #LEDs) = (" << data.maxSpeed << ", " << data.maxAngle << ", " << (uint16_t)data.maxBrightness << ", " << (uint16_t)data.numLeds << ")";
}

inline std::ostream& operator<<(std::ostream& out, const CommData::MoveData& data){
  return out << "(speed, angle) = (" << data.speed << ", " << data.angle << ")";
}

inline std::ostream& operator<<(std::ostream& out, const CommData::LedData& data){
  auto flags=out.flags();
  out << "RGB = 0x" << std::hex << data.red << data.green << data.blue;
  out.flags(flags);
  return out;
}
