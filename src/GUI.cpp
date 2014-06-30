#include <GUI.h>

#include <gtkmm/builder.h>
#include <glibmm/main.h>

#include <sstream>
#include <exception>
#include <iomanip>

class GUIException : public std::exception{
  private:
    const std::string msg;
  public:
    GUIException(const std::string& msg) throw() : msg(msg) {}
    virtual const char* what() const throw(){return msg.c_str();}
};

GUI::GUI(int argc, char** argv, const std::string& gladeFile, unsigned int fps) 
  : app(Gtk::Application::create(argc, argv, "Mobile Pi Remote Control")),
    mFps(fps){
  Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file(gladeFile);
  builder->get_widget("Window", mWindow);
  if(!mWindow)
    throw GUIException("Widget Window not found");
  builder->get_widget("V", mVLabel);
  if(!mVLabel)
    throw GUIException("Widget V not found");
  builder->get_widget("Theta", mThetaLabel);
  if(!mThetaLabel)
    throw GUIException("Widget Theta not found");
  builder->get_widget("Joystick", mJoystick);
  if(!mJoystick)
    throw GUIException("Widget Joystick not found");
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::onTimeout), 1000/mFps);
}

bool GUI::onTimeout(){
  std::ostringstream vOut;
  vOut << std::setprecision(2) << mControl.v;
  mVLabel->set_text(vOut.str());
  std::ostringstream thetaOut;
  thetaOut << std::setprecision(2) << mControl.theta;
  mThetaLabel->set_text(thetaOut.str());
  return true;
}

int GUI::run(){
  return app->run(*mWindow);
}
