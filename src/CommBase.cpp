#include <CommBase.h>

#include <sstream>
#include <boost/asio.hpp>
#include <boost/exception/exception.hpp>

using namespace boost::asio;
using namespace boost;
using boost::system::error_code;
using boost::system::system_error;
using boost::asio::ip::udp;
using std::stringstream;
using std::cerr;
using std::endl;
using std::chrono::milliseconds;

CommBase::CommBase(unsigned short port, milliseconds timeout)
  : mRunning(true),
    mSocket(mIos, udp::endpoint(udp::v4(), port)),
    mTimer(mIos),
    mIOThread([this](){runIOService();}),
    mTimeout(timeout) 
{}

CommBase::CommBase(const std::string& host, unsigned short port, milliseconds timeout)
  : mRunning(true),
    mSocket(mIos, udp::endpoint(udp::v4(), 0)),
    mTimer(mIos),
    mTimeout(timeout) {
  udp::resolver r(mIos);
  stringstream ss;
  ss << port;
  mEndpoint=*r.resolve({udp::v4(), host, ss.str()});
  mIOThread=std::thread([this](){runIOService();});
}

CommBase::~CommBase(){
  mRunning=false;
  mIos.stop();
  mIOThread.join();
}

void CommBase::startReceive(const ReceiveBuffers& buffers) {
  auto func = [this](const error_code& ec, size_t bytes){
    handleEvent(ec, bytes);
  };
  mSocket.async_receive_from(buffers, mEndpoint, func);
}

void CommBase::startTransmit(const TransmitBuffers& buffers) {
  auto func = [this](const error_code& ec, size_t bytes){
    handleEvent(ec, bytes);
  };
  mSocket.async_send_to(buffers, mEndpoint, func);
}

void CommBase::startTimer() {
  auto func = [this](const ErrorCode& e){
    if(e!=error::operation_aborted){
      startTimer();
      handleTimeout(e);
    }
  };
  mTimer.expires_from_now(mTimeout);
  mTimer.async_wait(func);
}

void CommBase::runIOService() throw() {
  io_service::work w(mIos);
  while(mRunning.load())
    try{
      mIos.run();
    }catch(std::exception& e){
      errorCallback(e);
    }
}

milliseconds CommBase::timeout() const {
  return mTimeout;
}

std::string CommBase::host() const {
  return mEndpoint.address().to_string();
}

uint16_t CommBase::port() const {
  return mEndpoint.port();
}

std::ostream& operator<<(std::ostream& out, const CommBase& data) {
  return out << data.host() << ":" << data.port() << " - " << data.timeout().count() << "ms";
}
