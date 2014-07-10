MACHINE  ?= $(shell ${CXX} -dumpmachine)

ifneq ($(findstring x86,${MACHINE}),)
	ARCH ?= x86
endif

ifneq ($(findstring arm,${MACHINE}),)
	ARCH ?= arm
endif

ifneq ($(findstring test,${MACHINE}),)
	ARCH ?= test
endif
