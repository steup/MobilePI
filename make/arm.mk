CXXFLAGS     += -march=armv6j 
LIBS         += bcm2835

ARCH_OBJECTS := ctrl Motor Servo CommReceiver

TARGET       := ctrl
