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

    /* \brief IO error type */
    using IOError = boost::system::system_error;

    /* \brief Error callback type */
    using ErrorHandlerType = boost::signals2::signal<void (CommError)>::slot_type;

  protected:

    using ReceiveBuffers = std::vector<boost::asio::mutable_buffer>;
    using TransmitBuffers = std::vector<boost::asio::const_buffer>;

  private:
    boost::asio::io_service mIos;
    boost::asio::ip::udp::socket mSocket;
    boost::asio::ip::udp::endpoint mEndpoint;
    boost::asio::steady_timer mTimer;
    boost::signals2::signal<void(CommError e)> errorCallback;
    std::thread mIOThread;
    std::chrono::milliseconds timeout;

    void runIOService() throw();
  protected:
    virtual void handleTimeout(const ErrorCode& e) throw(IOError, CommError) =0;
    virtual void handleEvent(const ErrorCode& e, std::size_t bytes) throw(IOError, CommError) =0;
    void startTimer() throw(IOError);
    void startReceive(const ReceiveBuffers& buffers) throw(IOError);
    void startTransmit(const TransmitBuffers& buffers) throw(IOError);
  public:


    /* \brief uncopiable*/
    CommBase(const CommBase&) = delete;

    /* \brief unassignable */
    CommBase& operator=(const CommBase&) = delete;

    /* \brief Sender constructor*/
    CommBase(const std::string& host, unsigned short port, unsigned long timeout) throw(IOError);

    /* \brief Receiver constructor*/
    CommBase(unsigned short port, unsigned long timeout) throw(IOError);

    /* \brief add an error handler
     * \param handler the error handler
     *
     * In case of exceptions during async operations this handler will be called
     */
    void addErrorHandler(ErrorHandlerType handler){errorCallback.connect(handler);}
};
