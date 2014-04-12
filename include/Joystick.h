#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

#include <boost/signals2/signal.hpp>

#include <SDL/SDL.h>

class Joystick{
  public:
    struct State{
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
    void addEventHandler(EventHandlerType handler);
    std::string name() const;
};
