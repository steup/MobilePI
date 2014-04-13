#include <CommReceiver.h>

#include <iostream>
#include <functional>

#include <boost/asio.hpp>

using boost::asio::mutable_buffer;
using std::vector;
using std::chrono::milliseconds;

CommReceiver::CommReceiver(unsigned short port, milliseconds timeout, uint8_t numLeds, uint8_t maxBrightness, int32_t maxSpeed, int32_t maxAngle) 
  throw(IOError)
  : CommBase(port, timeout), mCurBuf(0),
    mLeds({std::vector<CommData::LedData>(mInit.numLeds, CommData::LedData()),
        std::vector<CommData::LedData>(mInit.numLeds, CommData::LedData())}){
  startReceive(createReceiveBuffers());
  startTimer();
}

CommBase::ReceiveBuffers CommReceiver::createReceiveBuffers() throw(){
  unsigned int unusedBuf=(mCurBuf+1)%cNumBufs;
  CommBase::ReceiveBuffers buffers;

  buffers.push_back(mutable_buffer(&mMove[unusedBuf], sizeof(CommData::MoveData)));
  buffers.push_back(mutable_buffer(mLeds[unusedBuf].data(), mInit.numLeds*sizeof(CommData::LedData)));
  return buffers;
}

void CommReceiver::handleEvent(const CommBase::ErrorCode& e, size_t bytes) throw(CommError, IOError){
  if( bytes == ( sizeof(CommData::MoveData) + mInit.numLeds*sizeof(CommData::LedData) )){
    startTimer();
    mCurBuf=(mCurBuf.load()+1)%cNumBufs;
  }
  
  startReceive(createReceiveBuffers());
  
  if( e )
    throw IOError(e);

  if( bytes != ( sizeof(CommData::MoveData) + mInit.numLeds * sizeof(CommData::LedData) ))
    throw CommError(CommError::invalidData);

  eventCallback();
}

void CommReceiver::handleTimeout(const CommBase::ErrorCode& e) throw(IOError, CommError){
  throw CommError(CommError::timeout);
}

std::ostream& operator<<(std::ostream& out, const CommReceiver& recv){
  return out << "localhost <- " << static_cast<const CommBase&>(recv);
}
