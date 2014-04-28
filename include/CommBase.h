#pragma once
#include <CommError.h>

#include <thread>
#include <chrono>
#include <vector>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/system_error.hpp>
#include <boost/signals2/signal.hpp>

class CommBase{
  public:
    /* \brief Error code type */
    using ErrorCode = boost::system::error_code;

    /* \brief Error callback type */
    using ErrorHandlerType = boost::signals2::signal<void (std::exception&) throw()>::slot_type;

  protected:

    using ReceiveBuffers = std::vector<boost::asio::mutable_buffer>;
    using TransmitBuffers = std::vector<boost::asio::const_buffer>;

  private:
    boost::signals2::signal<void(std::exception& e) throw()> errorCallback;
    boost::asio::io_service mIos;
    boost::asio::ip::udp::socket mSocket;
    boost::asio::ip::udp::endpoint mEndpoint;
    boost::asio::steady_timer mTimer;
    std::thread mIOThread;
    std::chrono::milliseconds mTimeout;

    void runIOService() throw();
  protected:
    virtual void handleTimeout(const ErrorCode& e) =0;
    virtual void handleEvent(const ErrorCode& e, std::size_t bytes) =0;
    void startTimer();
    void startReceive(const ReceiveBuffers& buffers);
    void startTransmit(const TransmitBuffers& buffers);
  public:


    /* \brief uncopiable*/
    CommBase(const CommBase&) = delete;

    /* \brief unassignable */
    CommBase& operator=(const CommBase&) = delete;

    /* \brief Sender constructor*/
    CommBase(const std::string& host, unsigned short port, std::chrono::milliseconds timeout);

    /* \brief Receiver constructor*/
    CommBase(unsigned short port, std::chrono::milliseconds timeout);

    /* \brief add an error handler
     * \param handler the error handler
     *
     * In case of exceptions during async operations this handler will be called
     */
    void addErrorHandler(ErrorHandlerType handler){errorCallback.connect(handler);}
    std::chrono::milliseconds timeout() const;
    std::string host() const;
    uint16_t port() const;
};

std::ostream& operator<<(std::ostream& out, const CommBase& data);
