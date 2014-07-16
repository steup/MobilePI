CXXFLAGS     += -march=armv6j 
LIBS         += bcm2835

OBJECTS := ctrl Motor Servo CommReceiver CommBase

TARGET       := ctrl
