CXXFLAGS       := -Wall -Os -g -std=gnu++11 -pthread
LDFLAGS        := -O1 --as-needed
LD_PRE         := -pthread
INCLUDE        := ./include
BUILD          := ./build
SRC            := ./src
BIN            := ./bin
DOC            := ./doc
INCLUDES       := 
LIBS           := boost_system boost_program_options boost_filesystem
LDPATHS        :=

#ARCH           ?= test

include make/arch.mk
include make/${ARCH}.mk
include make/rules.mk
