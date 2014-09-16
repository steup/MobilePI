#include <GUI.h>
#include <Exception.h>

#include <gtkmm/label.h>
#include <gtkmm/window.h>
#include <glibmm/main.h>
#include <gtkmm/drawingarea.h>

#include <iomanip>
#include <iostream>
#include <cmath>

namespace {
  template<typename T>
  T& loadWidget(Glib::RefPtr<Gtk::Builder> builder, const std::string& name){
    T* widgetPtr = nullptr;
    builder->get_widget(name, widgetPtr);
    if(widgetPtr)
      return *widgetPtr;
    else
      throw Exception() << __PRETTY_FUNCTION__ << ": " << __LINE__ << " - Unable to find widget " << name << "!" << std::endl;
  }
}

GUI::GUI(int& argc, char**& argv, const std::string& gladeFile, unsigned int fps)
  : mStream(mArgCount, argv),
    mApp(Gtk::Application::create(mArgCount, argv)),
    mBuilder(Gtk::Builder::create_from_file(gladeFile)),
    mWindow(loadWidget<Gtk::Window>(mBuilder, "Window")),
    mCanvas(loadWidget<Gtk::DrawingArea>(mBuilder, "Canvas")),
    mVLabel(loadWidget<Gtk::Label>(mBuilder, "V")),
    mThetaLabel(loadWidget<Gtk::Label>(mBuilder, "Theta")),
    mJoystick(loadWidget<Gtk::Label>(mBuilder, "Joystick")),
    mFps(fps) {
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::onTimeout), 1000/mFps);
  mCanvas.signal_draw().connect(sigc::mem_fun(*this, &GUI::onDraw), false);
  mCanvas.signal_realize().connect(sigc::mem_fun(*this, &GUI::onRealize));
  mWindow.show_all();
}

void GUI::joystick( const std::string& name ) {
  mJoystick.set_text( name );
}

bool GUI::onTimeout(){
  std::ostringstream vOut;
  vOut << std::setprecision(2) << mControl.first;
  mVLabel.set_text(vOut.str());
  std::ostringstream thetaOut;
  thetaOut << std::setprecision(2) << mControl.second;
  mThetaLabel.set_text(thetaOut.str());
  mCanvas.queue_draw();
  return true;
}

bool GUI::onDraw(const Cairo::RefPtr<Cairo::Context>& cr){
  Gtk::Allocation allocation = mCanvas.get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();

  // Project canvas to unit square [-1,-1 ; 1,1] and turn positive y towards top
  cr->scale(width/2, height/2);
  cr->translate(1.0, 1.0);
  cr->rotate(M_PI);
  cr->save();
  // Clear the window with black background
  if(mStream.state() != VideoStream::play) {
    cr->set_source_rgba(0.0, 0.0, 0.0, 1.0);
    cr->paint();
    cr->clip();
    cr->restore();
  }
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
  return false;
}

void GUI::onRealize() {
  mStream.widget(mCanvas);
  mStream.uri("file:///home/ssj5kc/Person-of-Interest_2_1.mp4");
  mStream.state(VideoStream::play);
}
