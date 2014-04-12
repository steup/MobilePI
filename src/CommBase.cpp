#include <CommBase.h>

#include <sstream>
#include <boost/asio.hpp>

using namespace boost::asio;
using boost::system::error_code;
using boost::system::system_error;
using boost::asio::ip::udp;
using std::stringstream;
using std::cerr;
using std::endl;

CommBase::CommBase(unsigned short port, unsigned long timeout)
  throw(IOError)
  : mSocket(mIos, udp::endpoint(udp::v4(), port)),
    mTimer(mIos),
    mIOThread([this](){runIOService();}),
    timeout(timeout){
}

CommBase::CommBase(const std::string& host, unsigned short port, unsigned long timeout)
  throw(IOError)
  : mSocket(mIos, udp::endpoint(udp::v4(), 0)),
    mTimer(mIos),
    mIOThread([this](){runIOService();}),
    timeout(timeout){
  udp::resolver r(mIos);
  stringstream ss;
  ss << port;
  mEndpoint=*r.resolve({udp::v4(), host, ss.str()});
}

void CommBase::startReceive(const ReceiveBuffers& buffers) throw(IOError){
  auto func = [this](const error_code& ec, size_t bytes){
    handleEvent(ec, bytes);
  };
  mSocket.async_receive_from(buffers, mEndpoint, func);
}

void CommBase::startTransmit(const TransmitBuffers& buffers) throw(IOError){
  auto func = [this](const error_code& ec, size_t bytes){
    handleEvent(ec, bytes);
  };
  mSocket.async_send_to(buffers, mEndpoint, func);
}

void CommBase::startTimer() throw(IOError){
  auto func = [this](const ErrorCode& e){
    if(e!=error::operation_aborted){
      startTimer();
      handleTimeout(e);
    }
  };
  mTimer.expires_from_now(timeout);
  mTimer.async_wait(func);
}

void CommBase::runIOService() throw()
{
  io_service::work w(mIos);
  while(true){
    try{
      mIos.run();
    }
    catch(IOError& e){
     cerr << "IO error: " << e.code() << endl; 
    }
    catch(CommError& e){
      errorCallback(e);
    }
  }
}
