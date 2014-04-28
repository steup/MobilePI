#include <CommReceiver.h>
#include <CommError.h>

#include <boost/asio.hpp>

using boost::asio::mutable_buffer;
using namespace boost;
using std::vector;
using std::chrono::milliseconds;

CommReceiver::CommReceiver(unsigned short port, milliseconds timeout, uint8_t numLeds, uint8_t maxBrightness, int32_t maxSpeed, int32_t maxAngle) 
  : CommBase(port, timeout), mCurBuf(0),
    mLeds({std::vector<CommData::LedData>(mInit.numLeds, CommData::LedData()),
        std::vector<CommData::LedData>(mInit.numLeds, CommData::LedData())}){
  startReceive(createReceiveBuffers());
  startTimer();
}

CommBase::ReceiveBuffers CommReceiver::createReceiveBuffers(){
  unsigned int unusedBuf=(mCurBuf+1)%cNumBufs;
  CommBase::ReceiveBuffers buffers;

  buffers.push_back(mutable_buffer(&mMove[unusedBuf], sizeof(CommData::MoveData)));
  buffers.push_back(mutable_buffer(mLeds[unusedBuf].data(), mInit.numLeds*sizeof(CommData::LedData)));
  return buffers;
}

void CommReceiver::handleEvent(const CommBase::ErrorCode& e, size_t bytes){
  if( bytes == ( sizeof(CommData::MoveData) + mInit.numLeds*sizeof(CommData::LedData) )){
    startTimer();
    mCurBuf=(mCurBuf.load()+1)%cNumBufs;
  }
  
  startReceive(createReceiveBuffers());
  
  if( e )
    throw CommError::IOError() << CommError::IOErrorInfo(e)
                               << throw_function(__PRETTY_FUNCTION__)
                               << throw_file(__FILE__)
                               << throw_line(__LINE__);

  if( bytes != ( sizeof(CommData::MoveData) + mInit.numLeds * sizeof(CommData::LedData) ))
    throw CommError::InvalidData() << throw_function(__PRETTY_FUNCTION__)
                                   << throw_file(__FILE__)
                                   << throw_line(__LINE__);
  eventCallback();
}

void CommReceiver::handleTimeout(const CommBase::ErrorCode& e){
  throw CommError::Timeout() << throw_function(__PRETTY_FUNCTION__)
                             << throw_file(__FILE__)
                             << throw_line(__LINE__);
}

std::ostream& operator<<(std::ostream& out, const CommReceiver& recv){
  return out << "localhost <- " << static_cast<const CommBase&>(recv);
}
