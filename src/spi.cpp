#include <string>
#include <iostream>
#include <sstream>
#include <system_error>
#include <vector>

#include <cerrno>
#include <cstdint>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

class SpiDev {
  public:
    struct Mode {
     union {
      uint8_t mode; 
      struct {
        bool clockPhase    : 1;
        bool clockPolarity : 1;
        bool reserved0     : 1;
        bool lsbFirst      : 1;
        bool reserved1     : 1;
        bool loopback      : 1;
      };
     };
     Mode() = default;
     Mode(uint8_t mode) : mode(mode) {} 
     const char* name() { return "Mode"; }
    };
    struct Speed {
      uint32_t speed;
      Speed() = default;
      Speed(uint32_t speed) : speed(speed) {}
      const char* name() { return "Speed"; }
    };
    struct BitsPerWord {
      uint8_t bits;
      BitsPerWord() = default;
      BitsPerWord(uint8_t bits) : bits(bits) {}
      const char* name() { return "BitsPerWord"; }
    };
    struct BitAlignment {
      uint8_t align;
      BitAlignment() = default;
      BitAlignment(uint8_t align) : align(align) {}
      const char* name() { return "BitAlignment"; }
    };
    using Message = std::vector<uint8_t>;

  private:
    void attr(unsigned long, void* ptr, const char* attrName, bool set);
    
    const std::string mDevName;
    const int mDev;
  public:
    SpiDev(const std::string device);
    ~SpiDev();
    const std::string& device() const { return mDevName; }
    void set(Mode mode);
    void get(Mode& mode);
    void set(Speed speed);
    void get(Speed& speed);
    void set(BitsPerWord bits);
    void get(BitsPerWord& bits);
    void set(BitAlignment align);
    void get(BitAlignment& align);
    void read(Message& msg);
    void write(const Message& msg);
    void transfer(Message& msg);
};
    
SpiDev::SpiDev(const std::string device)
  bcm2835_spi_begin();
}

SpiDev::~SpiDev(){
  bcm2835_spi_end():
}

void SpiDev::set(Mode mode){
  bcm2835_spi_setDataMode(mode.mode);
}

void SpiDev::get(Mode& mode){
}

void SpiDev::set(Speed speed){
  bcm2835_spi_setClockDivider(BCMD2835_SPI_CLOCK_DIVIDER_512);
}

void SpiDev::get(Speed& speed){
}

void SpiDev::set(BitsPerWord bits){
}

void SpiDev::get(BitsPerWord& bits){
}

void SpiDev::set(BitAlignment align){
}

void SpiDev::get(BitAlignment& align){
}

void SpiDev::read(Message& msg){
 bcm2835_spi_readnb(msg.data(), msg.size());
}

void SpiDev::write(const Message& msg){
  bcm2835_spi_writenb(msg.data(), msg.size());
}

void SpiDev::transfer(Message& msg){
 Message recv = msg;
 bcm2835_spi_transfernb(msg.data(). recv.data(), msg.size());
 msg = recv;
}

std::ostream& operator<<(std::ostream& o, const SpiDev::Message& msg) {
  for(auto v : msg)
    o << v;
  return o;
}

int main(int argc, char** argv) {
  if(argc<2) {
    std::cout << "Usage: " << argv[0] << " <device name> <tx message>" << std::endl;
    return -1;
  }
  SpiDev spi(argv[1]);
  spi.set(SpiDev::Speed(500000));
  spi.set(SpiDev::BitsPerWord(8));
  spi.set(SpiDev::BitAlignment(0));
  SpiDev::Message msg;
  auto i=std::back_inserter(msg);
  *i++=0x81;
  while(*argv[2])
    *i++=*argv[2]++;
  std::cout << "Transfer " << msg << "(" << msg.size() << ") to SPI device " << spi.device() << std::endl;
  spi.transfer(msg);
  std::cout << "Answer was 0x" << std::hex << msg << std::endl;
  return 0;
}
