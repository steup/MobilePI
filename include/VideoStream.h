#include <string>
#include <iosfwd>

#include <exception>

#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/hvbox.h>
#include <gtkmm/hvscale.h>
#include <gtkmm/textview.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/main.h>

#include <gst/gst.h>

class Exception : public std::exception{
  private:
  std::string msg;
  public:
    template<typename T>
    Exception& operator<<(const T& value);
    Exception& operator<<(std::ostream& (*f)(std::ostream&));
    virtual const char* what() const throw(){ return msg.c_str();}
};
   
class GUI{
  private:
    int argc;
    char** argv;
    
    GstElement*       pipeline;
    GstBus*           msgBus;
    GstState          pipelineState;
    gint64            duration;

    Glib::RefPtr<Gtk::Application> app;
    Gtk::Window                    window;
    Gtk::DrawingArea               canvas;
    Gtk::HScale                    slider;
    Gtk::TextView                  streamInfo;
    Gtk::HBox                      toolbar, stream;
    Gtk::VBox                      layout;
    Gtk::Button                    play, pause, stop;

    sigc::connection  sliderConnection;


    void onRealize();
    void onPlay()  { gst_element_set_state (pipeline, GST_STATE_PLAYING); }
    void onPause() { gst_element_set_state (pipeline, GST_STATE_PAUSED);  }
    void onStop()  { gst_element_set_state (pipeline, GST_STATE_READY);   }
    bool onDraw(const Cairo::RefPtr<Cairo::Context>& cr);
    void onSeek();
    bool onTimeout();
    
    static void onNotify     ( GstBus* bus, GstMessage* msg, GUI* gui );
    static void onMetadata   ( GstBus* bus, GstMessage* msg, GUI* gui );
    static void onStateChange( GstBus* bus, GstMessage* msg, GUI* gui );
    static void onEndOfStream( GstBus* bus, GstMessage* msg, GUI* gui );
    static void onError      ( GstBus* bus, GstMessage* msg, GUI* gui );

  public:
    GUI(int argc, char** argv);
    ~GUI();
    void run() { app->run(window, argc, argv); }
};
