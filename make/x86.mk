PKG_NAME      := gtkmm-3.0
include make/pkg-config.mk

PKG_NAME      := gstreamer-1.0
include make/pkg-config.mk

PKG_NAME      := gstreamer-video-1.0
include make/pkg-config.mk

PKG_NAME      := sdl
include make/pkg-config.mk

CXXFLAGS      += -march=native

ARCH_OBJECTS  := remote Joystick CommSender GUI VideoStream

TARGET        := remote
