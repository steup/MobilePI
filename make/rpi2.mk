CC        := armv7a-hardfloat-linux-gnueabi-gcc
CXX       := armv7a-hardfloat-linux-gnueabi-g++
CXXFLAGS  += -march=armv7-a 
LIBS      += bcm2835

OBJECTS   := ctrl Motor Servo CommReceiver CommBase

TARGET    := ctrl
