#pragma once

#include <JoyError.h>

#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <ostream>

#include <boost/signals2/signal.hpp>

#include <SDL/SDL.h>
#include <limits>

class Joystick{
  public:
    struct State{
      static const int16_t axisMin=std::numeric_limits<int16_t>::min();
      static const int16_t axisMax=std::numeric_limits<int16_t>::max();
      std::vector<int16_t> axes;
      std::vector<bool> buttons;
    };

  private:
    using Lock=std::lock_guard<std::mutex>;

    std::mutex mMutex;
    SDL_Joystick* mJoyDev;
    unsigned int mJoyNum;
    boost::signals2::signal<void (const State&)> eventCallback;
    State mState;
    std::thread mThread;
    std::atomic<bool> mRunning;

    void handleJoystick();

  public:
    using EventHandlerType = boost::signals2::signal<void (const State&)>::slot_type;

    static std::vector<std::string> getAvailableJoysticks();
    static unsigned int getNumJoysticks();
    Joystick(unsigned int joyNum);
    ~Joystick();
    State getState();
    std::string name() const;
    void addEventHandler(EventHandlerType handler);
};

std::ostream& operator<<(std::ostream& out, const Joystick& j);
