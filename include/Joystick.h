#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

#include <boost/signals2/signal.hpp>

#include <SDL/SDL.h>
#include <limits>

class Joystick {
  public:
    struct State {
      static const int16_t axisMin = std::numeric_limits< int16_t >::min();
      static const int16_t axisMax = std::numeric_limits< int16_t >::max();
      std::vector< int16_t > axes;
      std::vector< bool > buttons;
    };

  private:
    using Lock = std::lock_guard< std::mutex >;
    using EventCallback = boost::signals2::signal< void (const State&) >;
    using ErrorCallback = boost::signals2::signal< void (const std::exception& e) >;

    static unsigned int sInitCount;
    std::mutex mMutex;
    SDL_Joystick* mJoyDev;
    unsigned int mJoyNum;
    EventCallback eventCallback;
    ErrorCallback errorCallback;
    State mState;
    std::thread mThread;
    std::atomic< bool > mRunning;

    void handleJoystick();

    static void init();
    static void uninit();

  public:
    using EventHandlerType = EventCallback::slot_type;
    using ErrorHandlerType = ErrorCallback::slot_type;

    static std::vector<std::string> getAvailableJoysticks();
    static unsigned int getNumJoysticks();
    Joystick(unsigned int joyNum);
    ~Joystick();
    State getState();
    std::string name() const;
    void addEventHandler(EventHandlerType handler);
    void addErrorHandler(ErrorHandlerType handler);
};

std::ostream& operator<<(std::ostream& out, const Joystick& j);
