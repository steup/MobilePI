#include <Joystick.h>

using namespace boost;

std::vector<std::string> Joystick::getAvailableJoysticks(){
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK))
      throw JoystickError::InitError() << JoystickError::SDLInfo(SDL_GetError())
                                       << throw_function(__PRETTY_FUNCTION__)
                                       << throw_file(__FILE__)
                                       << throw_line(__LINE__);
  std::vector<std::string> joys;
  for(int i=0;i<SDL_NumJoysticks();i++)
  {
    const char* name=SDL_JoystickName(i);
    if(!name)
      throw JoystickError::NoSuchJoystick() << JoystickError::NumberInfo(i)
                                            << throw_function(__PRETTY_FUNCTION__)
                                            << throw_file(__FILE__)
                                            << throw_line(__LINE__);
    joys.push_back(name);
  }
  SDL_Quit();
  return joys;
}

unsigned int Joystick::getNumJoysticks(){
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK))
      throw JoystickError::InitError() << JoystickError::SDLInfo(SDL_GetError())
                                       << throw_function(__PRETTY_FUNCTION__)
                                       << throw_file(__FILE__)
                                       << throw_line(__LINE__);
  unsigned int temp=SDL_NumJoysticks();
  SDL_Quit();
  return temp;
}

Joystick::Joystick(unsigned int joyNum) : 
  mJoyNum(joyNum),
  mRunning(true){
  Lock lock(mMutex);
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK))
      throw JoystickError::InitError() << JoystickError::SDLInfo(SDL_GetError())
                                       << throw_function(__PRETTY_FUNCTION__)
                                       << throw_file(__FILE__)
                                       << throw_line(__LINE__);
  mJoyDev=SDL_JoystickOpen(joyNum);
  if(!mJoyDev)
    throw JoystickError::NoSuchJoystick() << JoystickError::NumberInfo(mJoyNum)
                                          << throw_function(__PRETTY_FUNCTION__)
                                          << throw_file(__FILE__)
                                          << throw_line(__LINE__);
  mThread=std::thread([this](){handleJoystick();});
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

void Joystick::addErrorHandler(ErrorHandlerType handler){
  errorCallback.connect(handler);
}


void Joystick::handleJoystick(){
  while(mRunning.load()){
    try{
      SDL_Event e;
      if(!SDL_WaitEvent(&e))
        throw JoystickError::EventError() << JoystickError::SDLInfo(SDL_GetError())
                                          << throw_function(__PRETTY_FUNCTION__)
                                          << throw_file(__FILE__)
                                          << throw_line(__LINE__);
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
    catch(std::exception e){
      errorCallback(e);
      }
  }
}

std::string Joystick::name() const{
  const char* name=SDL_JoystickName(mJoyNum);
  if(!name)
    throw JoystickError::NoSuchJoystick() << JoystickError::NumberInfo(mJoyNum)
                                          << throw_function(__PRETTY_FUNCTION__)
                                          << throw_file(__FILE__)
                                          << throw_line(__LINE__);
  return name;
}

std::ostream& operator<<(std::ostream& out, const Joystick& j){
  return out << "Joystick: " << j.name();
}
