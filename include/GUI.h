#pragma once

#include <VideoStream.h>

#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <giomm/applicationcommandline.h>
#include <cairomm/context.h>
#include <string>
#include <utility>

namespace Gtk{
  class Window;
  class DrawingArea;
  class Label;
}

class GUI{
  public:
    using Control = std::pair<float, float>;

  private:
    int                            mArgCount = 1;
    VideoStream                    mStream;
    Glib::RefPtr<Gtk::Application> mApp;
    Glib::RefPtr<Gtk::Builder>     mBuilder;
    Gtk::Window&                   mWindow;
    Gtk::DrawingArea&              mCanvas;
    Gtk::Label&                    mVLabel;
    Gtk::Label&                    mThetaLabel;
    Gtk::Label&                    mJoystick;
    Control                        mControl;
    const unsigned int             mFps;
    
  public:
    GUI( int& argc, char**& argv, const std::string& gladeFile, unsigned int fps=30 );
    virtual ~GUI() { stop(); };
    GUI(const GUI&) = delete;
    GUI& operator=(const GUI&) = delete;
    void joystick( const std::string& name );
    int run() { return mApp->run( mWindow ); }
    void stop() { mApp->quit(); }
    void value( const Control& ctrl) { mControl = ctrl; }
    unsigned int fps() const { return mFps; }

  protected: 
    bool onTimeout();
    bool onDraw(const Cairo::RefPtr<Cairo::Context>& cr);
    int onCmd(const Glib::RefPtr<Gio::ApplicationCommandLine>& cmd);
    void onRealize();
};
