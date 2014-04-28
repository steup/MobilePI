#pragma once

#include <CommData.h>
#include <CommError.h>
#include <CommBase.h>

#include <mutex>
#include <ostream>
#include <chrono>

class CommSender : public CommBase{
  private:
    std::mutex mMutex;
    CommData::InitData mInit;
    CommData::MoveData mMove;
    std::vector<CommData::LedData> mLeds;

    virtual void handleTimeout(const ErrorCode& e);
    virtual void handleEvent(const ErrorCode& e, std::size_t bytes);
    CommBase::TransmitBuffers createTransmitBuffers() const;

  public:
    using Leds = std::vector<CommData::LedData>;
    using Move = CommData::MoveData;
    using Parameters = CommData::InitData;

    CommSender(const CommSender&) = delete;
    CommSender& operator=(const CommSender&) = delete;

    CommSender(const std::string& host, unsigned short port, std::chrono::milliseconds timeout);
    const Move& getMoveData() const throw(){return mMove;}
    const Leds& getLedData()  const throw(){return mLeds;}
    const Parameters& getParameters() const throw(){return mInit;}
    void setMoveData(const Move& move);
    void setLedsData(const Leds& leds);
};

std::ostream& operator<<(std::ostream& out, const CommSender& sender);
