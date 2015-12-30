CXXFLAGS     += -march=armv7a 
LIBS         += bcm2835

OBJECTS := ctrl Motor Servo CommReceiver CommBase

TARGET       := ctrl
