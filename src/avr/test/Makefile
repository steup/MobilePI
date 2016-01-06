CXX := avr-g++
LD  := avr-g++
CPY := avr-objcopy
FLS := avrdude

CXXFLAGS := -Wall -Os -g -mmcu=at90can128 -DF_CPU=16000000ULL -ffunction-sections -fno-exceptions
LDFLAGS  := -mmcu=at90can128 -nodevicelib -Wl,-O1 -Wl,-gc-sections
CPYFLAGS := -Oihex -j .data -j .text
FLSFLAGS := -p at90can128 -P usb -c jtag2 -U f:w:

.PHONY: all upload

all: test

test: test.o
	${LD} ${LDFLAGS} $^ -o $@

test.o: test.cpp
	${CXX} -c ${CXXFLAGS} $< -o $@

test.ihex: test
	${CPY} ${CPYFLAGS} $< $@

upload: test.ihex
	${FLS} ${FLSFLAGS}$<

clean:
	rm -f  test.ihex test.o test
