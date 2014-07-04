#pragma once

#include <gtkmm/label.h>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <cairomm/context.h>
#include <gtkmm/drawingarea.h>

#include <string>

class GUI{
  public:
    struct Control{
      float v;
      float theta;
      Control() : v(0.0f), theta(0.0f){}
      Control(float v, float theta) : v(v), theta(theta){}
      };
  private:
    Gtk::Label* mVLabel = NULL;
    Gtk::Label* mThetaLabel = NULL;
    Gtk::Label* mJoystick = NULL;
    Gtk::Window* mWindow = NULL;
    Gtk::DrawingArea* mCanvas = NULL;
    Glib::RefPtr<Gtk::Application> app;
    Control mControl;
    const unsigned int mFps;
  public:
    GUI(int argc, char** argv, const std::string& gladeFile, unsigned int fps=30);
    const Control& value() const { return mControl; }
    void value(const Control& ctrl) { mControl=ctrl; }
    unsigned int fps() const { return mFps; }
    void joystick(const std::string& name) {mJoystick -> set_text(name);}
    int run();

  protected:
    bool onTimeout();
    bool onDraw(const Cairo::RefPtr<Cairo::Context>& cr);
};
