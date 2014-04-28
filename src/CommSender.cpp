#include <CommSender.h>
#include <CommError.h>

#include <algorithm>

#include <boost/asio.hpp>

using boost::asio::const_buffer;
using namespace boost;
using std::chrono::milliseconds;

CommSender::CommSender(const std::string& host, unsigned short port, milliseconds timeout)
  : CommBase(host, port, timeout),
    mLeds(mInit.numLeds, CommData::LedData()){
  startTimer();
}

void CommSender::setMoveData(const Move& move){
  mMutex.lock();
  mMove=move;
  startTransmit(createTransmitBuffers());
}

void CommSender::setLedsData(const Leds& leds){
  if(leds.size()>mLeds.size())
    throw CommError::LedError() << CommError::LedInfo(leds.size()-1)
                                << throw_function(__PRETTY_FUNCTION__)
                                << throw_file(__FILE__)
                                << throw_line(__LINE__);
  mMutex.lock();
  std::copy(leds.begin(), leds.end(), mLeds.begin());
  startTransmit(createTransmitBuffers());
}

void CommSender::handleTimeout(const ErrorCode& e){
  mMutex.lock();
  startTransmit(createTransmitBuffers());
}

void CommSender::handleEvent(const ErrorCode& e, std::size_t bytes){
  mMutex.unlock();
  startTimer();
}

CommBase::TransmitBuffers CommSender::createTransmitBuffers() const{
  CommBase::TransmitBuffers buffers;

  buffers.push_back(const_buffer(&mMove, sizeof(mMove)));
  buffers.push_back(const_buffer(mLeds.data(), mLeds.size()*sizeof(CommData::LedData)));

  return buffers;
}

std::ostream& operator<<(std::ostream& out, const CommSender& sender){
  return out << "localhost -> " << static_cast<const CommBase&>(sender);
}
