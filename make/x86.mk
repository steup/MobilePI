GTKMM_VERSION ?= 3.0

CXXFLAGS      += -march=native $(shell pkg-config gtkmm-${GTKMM_VERSION} --cflags)

LIBS          += SDL
LD_POST       += $(shell pkg-config gtkmm-${GTKMM_VERSION} --libs)

ARCH_OBJECTS  := remote Joystick CommSender GUI

TARGET        := remote
