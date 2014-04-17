#pragma once
#include <CommData.h>
#include <CommError.h>
#include <CommBase.h>

#include <cstdint>
#include <atomic>
#include <chrono>
#include <ostream>

#include <boost/signals2/signal.hpp>

class CommReceiver : public CommBase{
  public:
        /* \brief Event callback type */
    using EventHandlerType = boost::signals2::signal<void()>::slot_type;

  private:
    static const unsigned int cNumBufs=2;
    std::atomic<unsigned int> mCurBuf;
    CommData::InitData mInit;
    CommData::MoveData mMove[cNumBufs];
    std::vector<CommData::LedData> mLeds[cNumBufs];
    boost::signals2::signal<void()> eventCallback;

    virtual void handleEvent(const CommBase::ErrorCode& e, std::size_t bytes) throw(IOError, CommError);
    virtual void handleTimeout(const CommBase::ErrorCode& e) throw(IOError, CommError);
    CommBase::ReceiveBuffers createReceiveBuffers() throw();

  public:
    using Leds = std::vector<CommData::LedData>;
    using Move = CommData::MoveData;
    using Parameters = CommData::InitData;

    CommReceiver(const CommReceiver&) = delete;
    CommReceiver& operator=(const CommReceiver&) = delete;

    CommReceiver(unsigned short port, std::chrono::milliseconds timeout,
                   uint8_t numLeds, uint8_t maxBrightness, int32_t maxSpeed, 
                   int32_t maxAngle) throw(IOError);
    Move getMoveData() const throw(){return mMove[mCurBuf.load()];}
    Leds getLedData()  const throw(){return mLeds[mCurBuf.load()];}

    /* \brief add an event handler
     * \param handler the event handler
     *
     * This handler will be called whenever an event happened
     */
    void addEventHandler(EventHandlerType handler){eventCallback.connect(handler);}
    const Parameters& getParameters() const throw(){return mInit;}
};

std::ostream& operator<<(std::ostream& out, const CommReceiver& recv);
