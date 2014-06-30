TARGETS        := ctrl remote

CXXFLAGS       := -Wall -Os -g -std=gnu++11 -pthread
LDFLAGS        := -O1
LD_PRE         := -pthread
COMMON_OBJECTS := CommBase
INCLUDE        := ./include
BUILD          := ./build
SRC            := ./src
BIN            := ./bin
INCLUDES       := ${INCLUDE}
LIBS           := boost_system boost_program_options boost_filesystem
LDPATHS        :=

include make/arch.mk
include make/${ARCH}.mk
include make/rules.mk
