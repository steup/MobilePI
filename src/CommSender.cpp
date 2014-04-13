#include <CommSender.h>

#include <boost/asio.hpp>

using boost::asio::const_buffer;
using std::chrono::milliseconds;

CommSender::CommSender(const std::string& host, unsigned short port, milliseconds timeout) throw(IOError):
  CommBase(host, port, timeout),
  mLeds(mInit.numLeds, CommData::LedData()){
  startTimer();
}

void CommSender::setMoveData(Move move) throw(){
  mMutex.lock();
  mMove=move;
  startTransmit(createTransmitBuffers());
}

void CommSender::setLedsData(Leds leds) throw(){
  mMutex.lock();
  mLeds=leds;
  startTransmit(createTransmitBuffers());
}

void CommSender::handleTimeout(const ErrorCode& e) throw(IOError, CommError){
  mMutex.lock();
  startTransmit(createTransmitBuffers());
  throw CommError(CommError::timeout);
}

void CommSender::handleEvent(const ErrorCode& e, std::size_t bytes) throw(IOError, CommError){
  mMutex.unlock();
  startTimer();
}

CommBase::TransmitBuffers CommSender::createTransmitBuffers() const throw(){
  CommBase::TransmitBuffers buffers;

  buffers.push_back(const_buffer(&mMove, sizeof(mMove)));
  buffers.push_back(const_buffer(mLeds.data(), mLeds.size()*sizeof(CommData::LedData)));

  return buffers;
}

std::ostream& operator<<(std::ostream& out, const CommSender& sender){
  return out << "localhost -> " << static_cast<const CommBase&>(sender);
}
