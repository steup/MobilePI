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
    
void SpiDev::attr(unsigned long req, void* ptr, const char* attrName, bool set){
  if(ioctl(mDev, req, ptr)==-1) {
    std::ostringstream os;
    os << "Error " << (set?"s":"g") << "etting SPI attribute " << attrName << " of device " << mDevName;
    throw std::system_error(errno, std::system_category(), os.str());
  }
}

SpiDev::SpiDev(const std::string device)
  : mDevName(device), mDev(open(device.c_str(), O_RDWR)) {
  if(mDev==-1) {
    std::ostringstream os;
    os << "Error opening SPI device " << mDevName;
    throw std::system_error(errno, std::system_category(), os.str());
  }
}

SpiDev::~SpiDev(){
  if(::close(mDev)==-1) {
    std::ostringstream os;
    os << "Error closing SPI device " << mDevName;
    throw std::system_error(errno, std::system_category(), os.str());
  }   
}

void SpiDev::set(Mode mode){
  attr(SPI_IOC_WR_MODE, &mode.mode, mode.name(), true);
}

void SpiDev::get(Mode& mode){
  attr(SPI_IOC_RD_MODE, &mode.mode, mode.name(), false);
}

void SpiDev::set(Speed speed){
  attr(SPI_IOC_WR_MAX_SPEED_HZ, &speed.speed, speed.name(), true);
}

void SpiDev::get(Speed& speed){
  attr(SPI_IOC_RD_MAX_SPEED_HZ, &speed.speed, speed.name(), false);
}

void SpiDev::set(BitsPerWord bits){
  attr(SPI_IOC_WR_BITS_PER_WORD, &bits.bits, bits.name(), true);
}

void SpiDev::get(BitsPerWord& bits){
  attr(SPI_IOC_RD_BITS_PER_WORD, &bits.bits, bits.name(), false);
}

void SpiDev::set(BitAlignment align){
  attr(SPI_IOC_WR_LSB_FIRST, &align.align, align.name(), true);
}

void SpiDev::get(BitAlignment& align){
  attr(SPI_IOC_RD_LSB_FIRST, &align.align, align.name(), false);
}

void SpiDev::read(Message& msg){
 if(::read(mDev, msg.data(), msg.size())!=(ssize_t)msg.size()) {
    std::ostringstream os;
    os << "Error reading from SPI device " << mDevName << " a message of size " << msg.size();
    throw std::system_error(std::error_code(errno, std::system_category()), os.str());
  }   
}

void SpiDev::write(const Message& msg){
 if(::write(mDev, msg.data(), msg.size())!=(ssize_t)msg.size()) {
    std::ostringstream os;
    os << "Error writing to SPI device " << mDevName << " a message of size " << msg.size();
    throw std::system_error(errno, std::system_category(), os.str());
 }
}

void SpiDev::transfer(Message& msg){
 spi_ioc_transfer trans;
 trans.tx_buf = (unsigned long)msg.data();
 trans.rx_buf = (unsigned long)msg.data();
 trans.len    = msg.size();
 if(::ioctl(mDev, SPI_IOC_MESSAGE(1), &trans)==-1){
    std::ostringstream os;
    os << "Error read/write with SPI device " << mDevName << " a transaction of size " << msg.size();
    throw std::system_error(errno, std::system_category(), os.str());
 }
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
  SpiDev::Mode mode;
  mode.clockPolarity = false;
  mode.clockPhase    = false;
  mode.lsbFirst      = false;
  mode.loopback      = true;
  spi.set(mode);
  spi.set(SpiDev::Speed(500000));
  spi.set(SpiDev::BitsPerWord(8));
  spi.set(SpiDev::BitAlignment(0));
  SpiDev::Message msg;
  auto i=std::back_inserter(msg);
  while(*argv[2])
    *i++=*argv[2]++;
  std::cout << "Transfer " << msg << " to SPI device " << spi.device() << std::endl;
  spi.transfer(msg);
  std::cout << "Answer was 0x" << std::hex << msg << std::endl;
  return 0;
}
