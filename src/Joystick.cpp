#include <Joystick.h>

std::vector<std::string> Joystick::getAvailableJoysticks(){
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  std::vector<std::string> joys;
  for(int i=0;i<SDL_NumJoysticks();i++)
    joys.push_back(SDL_JoystickName(i));
  SDL_Quit();
  return joys;
}

unsigned int Joystick::getNumJoysticks(){
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  unsigned int temp=SDL_NumJoysticks();
  SDL_Quit();
  return temp;
}

Joystick::Joystick(unsigned int joyNum) : 
  mJoyNum(joyNum),
  mThread([this](){handleJoystick();}),
  mRunning(true){
  Lock lock(mMutex);
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  mJoyDev=SDL_JoystickOpen(joyNum);
  SDL_JoystickEventState(SDL_ENABLE);
  SDL_JoystickUpdate();
  mState.axes.resize(SDL_JoystickNumAxes(mJoyDev));
  mState.buttons.resize(SDL_JoystickNumButtons(mJoyDev));
}

Joystick::~Joystick(){
  mRunning=false;
  SDL_JoystickClose(mJoyDev);
  SDL_Quit();
  mThread.join();
}

Joystick::State Joystick::getState(){
  Lock lock(mMutex);
  return mState;
}

void Joystick::addEventHandler(EventHandlerType handler){
  eventCallback.connect(handler);
}

void Joystick::handleJoystick(){
  while(mRunning.load()){
    SDL_Event e;
    SDL_WaitEvent(&e);
    Lock lock(mMutex);
    switch(e.type){
      case(SDL_JOYAXISMOTION): mState.axes[e.jaxis.axis]=e.jaxis.value;
                               eventCallback(mState);
                               break;
      case(SDL_JOYBUTTONDOWN):
      case(SDL_JOYBUTTONUP):   mState.buttons[e.jbutton.button]=e.jbutton.state;
                               eventCallback(mState);
      }
  }
}

std::string Joystick::name() const{
  return SDL_JoystickName(mJoyNum);
}
