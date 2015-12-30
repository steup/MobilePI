MACHINE  ?= $(shell ${CXX} -dumpmachine)

ifneq ($(findstring x86,${MACHINE}),)
	ARCH ?= x86
endif

ifneq ($(findstring armv7,${MACHINE}),)
	ARCH ?= rpi2
endif

ifneq ($(findstring armv6,${MACHINE}),)
	ARCH ?= rpi
endif

ifneq ($(findstring test,${MACHINE}),)
	ARCH ?= test
endif
