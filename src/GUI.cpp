#include <GUI.h>

#include <gtkmm/label.h>
#include <gtkmm/window.h>
#include <glibmm/main.h>
#include <gtkmm/drawingarea.h>

#include <sstream>
#include <exception>
#include <iomanip>
#include <cmath>

class GUIException : public std::exception{
  private:
    const std::string msg;
  public:
    GUIException(const std::string& msg) throw() : msg(msg) {}
    virtual const char* what() const throw(){return msg.c_str();}
};

GUI::GUI(const std::string& gladeFile, unsigned int fps)
  : mApp(Gtk::Application::create()),
    mBuilder(Gtk::Builder::create_from_file(gladeFile)),
    mFps(fps){
  mBuilder->get_widget("Window", mWindow);
  if(!mWindow)
    throw GUIException("Widget Window not found");
  mBuilder->get_widget("V", mVLabel);
  if(!mVLabel)
    throw GUIException("Widget V not found");
  mBuilder->get_widget("Theta", mThetaLabel);
  if(!mThetaLabel)
    throw GUIException("Widget Theta not found");
  mBuilder->get_widget("Joystick", mJoystick);
  if(!mJoystick)
    throw GUIException("Widget Joystick not found");
  mBuilder->get_widget("Canvas", mCanvas);
  if(!mCanvas)
    throw GUIException("Widget Canvas not found");
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::onTimeout), 1000/mFps);
  mCanvas->signal_draw().connect(sigc::mem_fun(*this, &GUI::onDraw), false);
  mWindow->show_all();
}

void GUI::joystick( const std::string& name ) {
  mJoystick -> set_text( name );
}

bool GUI::onTimeout(){
  std::ostringstream vOut;
  vOut << std::setprecision(2) << mControl.first;
  mVLabel->set_text(vOut.str());
  std::ostringstream thetaOut;
  thetaOut << std::setprecision(2) << mControl.second;
  mThetaLabel->set_text(thetaOut.str());
  mCanvas->queue_draw();
  return true;
}

bool GUI::onDraw(const Cairo::RefPtr<Cairo::Context>& cr){
  Gtk::Allocation allocation = mCanvas->get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();

  // Project canvas to unit square [-1,-1 ; 1,1] and turn positive y towards top
  cr->scale(width/2, height/2);
  cr->translate(1.0, 1.0);
  cr->rotate(M_PI);
  cr->save();
  // Clear the window with black background
  cr->set_source_rgba(0.0, 0.0, 0.0, 1.0);
  cr->paint();
  cr->clip();
  cr->restore();
  // Set drawing color to white
  cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
  cr->save();

  if(mControl.first<0.001) 
  {
    // No movement, show dot
    cr->arc(0.0, 0.0, 0.05, 0, 2*M_PI-0.001);
    cr->fill();
  }else{
    // Rotate and scale arrow
    cr->rotate(mControl.second*M_PI*40/180);
    cr->scale(mControl.first, mControl.first);
    // Draw arrow
    cr->set_line_width(0.01);
    cr->move_to(0.0, 0.0);
    cr->line_to(0.0, 1.0);
    cr->rel_line_to(-0.25, -0.25);
    cr->stroke();
    cr->move_to(0.0,1.0);
    cr->rel_line_to(0.25, -0.25);
    cr->stroke();
  }
  return true;
}
