#include <string>
#include <iostream>
#include <sstream>
#include <exception>

#include <boost/filesystem.hpp>

#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/hvbox.h>
#include <gtkmm/hvscale.h>
#include <gtkmm/textview.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/main.h>

#include <glibmm/main.h>

#include <gdk/gdkx.h>

#include <gst/gst.h>
#include <gst/video/videooverlay.h>


using namespace boost::filesystem;
using namespace std;

class Exception : public exception{
  private:
  string msg;
  public:
    template<typename T>
    Exception& operator<<(const T& value){
      ostringstream o;
      o << value;
      msg += o.str();
      return *this;
    }
    Exception& operator<<(ostream& (*f)(ostream&)){
      ostringstream o;
      o << f;
      msg += o.str();
      return *this;
    }
    virtual const char* what() const throw(){ return msg.c_str();}
};
   
class GUI{
  private:
    int argc;
    char** argv;
    
    GstElement*       pipeline;           /* Our one and only pipeline */
    GstBus*           msgBus;
    GstState          pipelineState;      /* Current state of the pipeline */
    gint64            duration;           /* Duration of the clip, in nanoseconds */

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

/* This function is called when the GUI toolkit creates the physical window that will hold the video.
 * At this point we can retrieve its handler (which has a different meaning depending on the windowing system)
 * and pass it to GStreamer through the XOverlay interface. */
void GUI::onRealize() {
  auto nativeWindow = canvas.get_window();
   
  if (!nativeWindow->ensure_native())
    throw Exception() << __PRETTY_FUNCTION__ << ": Couldn't create native window needed for video overlay!" << endl;
   
  /* Pass it to playbin, which implements  video overlay and will forward it to the video sink */
  gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY (pipeline), GDK_WINDOW_XID(nativeWindow->gobj()));
}
   
/* This function is called everytime the video window needs to be redrawn (due to damage/exposure,
 * rescaling, etc). GStreamer takes care of this in the PAUSED and PLAYING states, otherwise,
 * we simply draw a black rectangle to avoid garbage showing up. */
bool GUI::onDraw(const Cairo::RefPtr<Cairo::Context>& cr) {
  if (pipelineState < GST_STATE_PAUSED) {
    auto allocation = canvas.get_allocation();
    cr->set_source_rgb( 0, 0, 0 );
    cr->rectangle( 0, 0, allocation.get_width(), allocation.get_height() );
    cr->fill();
  }   
  return false;
}
   
/* This function is called when the slider changes its position. We perform a seek to the
 * new position here. */
void GUI::onSeek() {
  gst_element_seek_simple( pipeline,
                           GST_FORMAT_TIME,
                           (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                           (gint64)(slider.get_value() * GST_SECOND)
  );
}
   
/* This creates all the GTK+ widgets that compose our application, and registers the callbacks */
GUI::GUI(int argc, char** argv)
  : argc(argc),
    argv(argv),
    pipeline(nullptr),
    duration(GST_CLOCK_TIME_NONE),
    app(Gtk::Application::create("GStreamer GTKmm Test")),
    slider(0, 100, 1){
  
  if(this->argc-- <=1)
    throw Exception() << "usage: " << argv[0] << " <file>" << endl;

  auto fileName = canonical(argv[1]);

  string uri = "file://";
  uri += fileName.native();

  cout << "playing file: " << fileName << endl;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  pipeline = gst_element_factory_make("playbin", "playbin");

  if(!pipeline)
    throw Exception() << __PRETTY_FUNCTION__ << ": Could not create playbin pipeline" << endl;

  /* Set the URI to play */
  g_object_set(pipeline, "uri", uri.c_str(), NULL);
   
  /* Connect to interesting signals in playbin2 */
  g_signal_connect( G_OBJECT( pipeline ), "video-tags-changed", (GCallback)&onMetadata, this);
  g_signal_connect( G_OBJECT( pipeline ), "audio-tags-changed", (GCallback)&onMetadata, this);
  g_signal_connect( G_OBJECT( pipeline ), "text-tags-changed" , (GCallback)&onMetadata, this);
   
  /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
  GstBus* bus = gst_element_get_bus( pipeline );
  gst_bus_add_signal_watch( bus );
  g_signal_connect( G_OBJECT( bus ), "message::error"        , (GCallback)&onError      , this );
  g_signal_connect( G_OBJECT( bus ), "message::eos"          , (GCallback)&onEndOfStream, this );
  g_signal_connect( G_OBJECT( bus ), "message::state-changed", (GCallback)&onStateChange, this );
  g_signal_connect( G_OBJECT( bus ), "message::application"  , (GCallback)&onNotify     , this );
  gst_object_unref( bus );
   
  /* Start playing */
  if ( gst_element_set_state( pipeline, GST_STATE_PLAYING ) == GST_STATE_CHANGE_FAILURE )
    throw Exception() << __PRETTY_FUNCTION__ << ": Unable to set the pipeline to the playing state." << endl;
   
  /* Register a function that GLib will call every second */
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::onTimeout), 1000);
   
  canvas.set_double_buffered( false);
  canvas.signal_realize().connect(sigc::mem_fun(*this, &GUI::onRealize));
  canvas.signal_draw().connect(sigc::mem_fun(*this, &GUI::onDraw));

  play.set_image_from_icon_name("media-playback-start", Gtk::ICON_SIZE_LARGE_TOOLBAR);
  play.signal_clicked().connect(sigc::mem_fun(*this, &GUI::onPlay));

  pause.set_image_from_icon_name("media-playback-pause", Gtk::ICON_SIZE_LARGE_TOOLBAR);
  pause.signal_clicked().connect(sigc::mem_fun(*this, &GUI::onPause));

  stop.set_image_from_icon_name("media-playback-stop", Gtk::ICON_SIZE_LARGE_TOOLBAR);
  stop.signal_clicked().connect(sigc::mem_fun(*this, &GUI::onStop));

  slider.set_draw_value( false );
  sliderConnection = slider.signal_value_changed().connect(sigc::mem_fun(*this, &GUI::onSeek));
   
  streamInfo.set_editable( false );
   
  toolbar.pack_start( play  , false, false, 2);
  toolbar.pack_start( pause , false, false, 2);
  toolbar.pack_start( stop  , false, false, 2);
  toolbar.pack_start( slider, true , true , 2);
   
  stream.pack_start( canvas    , true , true , 0);
  stream.pack_start( streamInfo, false, false, 2);

  layout.pack_start( stream , true, true, 0);
  layout.pack_start( toolbar, true, true, 0);
  
  window.add( layout );
  window.set_default_size( 640, 480 );
  window.show_all();
}
   
/* This function is called periodically to refresh the GUI */
bool GUI::onTimeout(){
  GstFormat fmt = GST_FORMAT_TIME;
   
  /* We do not want to update anything unless we are in the PAUSED or PLAYING states */
  if (pipelineState < GST_STATE_PAUSED)
    return true;
   
  /* If we didn't know it yet, query the stream duration */
  if (!GST_CLOCK_TIME_IS_VALID (duration)) {
    if (!gst_element_query_duration (pipeline, fmt, &duration)) {
      cerr <<  "Could not query current duration." << endl;
    } else {
      /* Set the range of the slider to the clip duration, in SECONDS */
      slider.set_range(0, (double)duration / GST_SECOND);
    }
  }

  gint64 current = -1;
  if (gst_element_query_position (pipeline, fmt, &current)) {
    /* Block the "value-changed" signal, so the slider_cb function is not called
     * (which would trigger a seek the user has not requested) */
    sliderConnection.block();
    /* Set the position of the slider to the current pipeline positoin, in SECONDS */
    slider.set_value((double)current / GST_SECOND);
    /* Re-enable the signal */
    sliderConnection.unblock();
  }
  return true;
}
   
/* This function is called when new metadata is discovered in the stream */
void GUI::onMetadata(GstBus* bus, GstMessage* stream, GUI *gui) {
  /* We are possibly in a GStreamer working thread, so we notify the main
   * thread of this event through a message in the bus */
  gst_element_post_message (gui->pipeline,
    gst_message_new_application (GST_OBJECT (gui->pipeline),
      gst_structure_new ("tags-changed", NULL)));
}
   
/* This function is called when an error message is posted on the bus */
void GUI::onError (GstBus *bus, GstMessage *msg, GUI *gui) {
  GError *err;
  gchar *debug_info;
   
  /* Print error details on the screen */
  gst_message_parse_error (msg, &err, &debug_info);
  cerr << "Error received from element " << GST_OBJECT_NAME (msg->src) << ":" << err->message << endl;
  cerr << "Debugging information: " << (debug_info ? debug_info : "none") << endl;
  g_clear_error (&err);
  g_free (debug_info);
   
  /* Set the pipeline to READY (which stops playback) */
  gst_element_set_state (gui->pipeline, GST_STATE_READY);
}
   
/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
void GUI::onEndOfStream(GstBus *bus, GstMessage *msg, GUI *gui) {
  cout << "End-Of-Stream reached." << endl;
  gst_element_set_state (gui->pipeline, GST_STATE_READY);
}
   
/* This function is called when the pipeline changes states. We use it to
 * keep track of the current state. */
void GUI::onStateChange(GstBus *bus, GstMessage *msg, GUI *gui) {
  GstState oldS, newS, pendingS;
  gst_message_parse_state_changed (msg, &oldS, &newS, &pendingS);
  if (GST_MESSAGE_SRC (msg) == GST_OBJECT (gui->pipeline)) {
    gui->pipelineState = newS;
    cout << "State set to " << gst_element_state_get_name (newS) << endl;
    if (oldS == GST_STATE_READY && newS == GST_STATE_PAUSED)
      /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
      gui->onTimeout();
  }
}
   
/* This function is called when an "application" message is posted on the bus.
 * Here we retrieve the message posted by the tags_cb callback */
void GUI::onNotify(GstBus *bus, GstMessage *msg, GUI *gui) {
  if (string("tags-changed") == gst_structure_get_name(gst_message_get_structure(msg))) {
    /* If the message is the "tags-changed" (only one we are currently issuing), update
     * the stream info GUI */

    ostringstream o;
    gint n_video, n_audio, n_text;
        
    /* Read some properties */
    g_object_get (gui->pipeline, "n-video", &n_video, NULL);
    g_object_get (gui->pipeline, "n-audio", &n_audio, NULL);
    g_object_get (gui->pipeline, "n-text", &n_text, NULL);
     
    for (int i = 0; i < n_video; i++) {
      GstTagList *tags = NULL;
      /* Retrieve the stream's video tags */
      g_signal_emit_by_name (gui->pipeline, "get-video-tags", i, &tags);
      if (tags) {
        gchar *str = NULL;

        gst_tag_list_get_string (tags, GST_TAG_VIDEO_CODEC, &str);
        o << "video stream " << i << ":" << endl;
        o << "  codec: " << (str ? str : "unknown") << endl;
        g_free (str);
        gst_tag_list_free (tags);
      }
    }
     
    for (int i = 0; i < n_audio; i++) {
      GstTagList *tags = NULL;
      /* Retrieve the stream's audio tags */
      g_signal_emit_by_name (gui->pipeline, "get-audio-tags", i, &tags);
      if (tags) {
        gchar* str = NULL;
        guint rate = 0;

        o << endl << "audio stream " << i << ":" << endl;
        
        if (gst_tag_list_get_string (tags, GST_TAG_AUDIO_CODEC, &str)) {
          o << "  codec: " << str << endl;
          g_free (str);
        }
        if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_CODE, &str)) {
          o << "  language: " << str << endl;
          g_free (str);
        }
        if (gst_tag_list_get_uint (tags, GST_TAG_BITRATE, &rate)) {
          o << "  bitrate: " << rate << endl;
        }
        gst_tag_list_free (tags);
      }
    }
     
    for (int i = 0; i < n_text; i++) {
      GstTagList *tags = NULL;
      /* Retrieve the stream's subtitle tags */
      g_signal_emit_by_name (gui->pipeline, "get-text-tags", i, &tags);
      if (tags) {
        gchar* str = NULL;
        o << endl << "subtitle stream " << i << ":" << endl;
        if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_CODE, &str)) {
          o << "  language: " << str << endl;
          g_free (str);
        }
        gst_tag_list_free (tags);
      }
    }
    gui->streamInfo.get_buffer()->set_text(o.str());
  }
}

/* Free resources */
GUI::~GUI(){
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
}

int main(int argc, char *argv[]) {
  try{
    GUI gui(argc, argv);
    gui.run();
  }
  catch(const exception& e){
    cerr << "Exception: " << e.what() << endl;
    return -1;
  }

  return 0;
}
