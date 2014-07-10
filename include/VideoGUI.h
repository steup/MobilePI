#pragma once

#include <VideoStream.h>

#include <string>

#include <gtkmm/drawingarea.h>
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/hvscale.h>
#include <glibmm/refptr.h>
#include <cairomm/refptr.h>
#include <sigc++/connection.h>

namespace Gtk{
  class Window;
  class TextView;
}

namespace Cairo{
  class Context;
}

class VideoGUI{
  private:
    std::string                    uri;
    VideoStream                    stream;
    Glib::RefPtr<Gtk::Application> app;
    Glib::RefPtr<Gtk::Builder>     builder;
    Gtk::Window&                   window;
    Gtk::DrawingArea&              canvas;
    Gtk::Scale&                    seek;
    Gtk::TextView&                 info;
    sigc::connection               seekConnection;

    void onRealize() { stream.widget(canvas);             }
    void onPlay()    { stream.state(VideoStream::play);   }
    void onPause()   { stream.state(VideoStream::pause);  }
    void onStop()    { stream.state(VideoStream::stop);   }
    bool onDraw(const Cairo::RefPtr<Cairo::Context>& cr);
    void onSeek()    { stream.position(seek.get_value()); }
    bool onTimeout();

    template<typename T>
    T& loadWidget(const std::string& name);

  public:
    VideoGUI(int argc, char** argv, const std::string& gladeFile);
    void run();
    void file(const std::string& file);
};
