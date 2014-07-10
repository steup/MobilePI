#include <VideoStream.h>

#include <sstream>
#include <iostream>

#include <boost/filesystem.hpp>

#include <glibmm/main.h>

#include <gdk/gdkx.h>

#include <gst/video/videooverlay.h>

using namespace boost::filesystem;
using namespace std;

template<typename T>
Exception& Exception::operator<<(const T& value){
  ostringstream o;
  o << value;
  msg += o.str();
  return *this;
}

Exception& Exception::operator<<(ostream& (*f)(ostream&)){
  ostringstream o;
  o << f;
  msg += o.str();
  return *this;
}

void GUI::onRealize() {
  auto nativeWindow = canvas.get_window();
   
  if (!nativeWindow->ensure_native())
    throw Exception() << __PRETTY_FUNCTION__ << ": Couldn't create native window needed for video overlay!" << endl;
   
  gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY (pipeline), GDK_WINDOW_XID(nativeWindow->gobj()));
}
   
bool GUI::onDraw(const Cairo::RefPtr<Cairo::Context>& cr) {
  if (pipelineState < GST_STATE_PAUSED) {
    auto allocation = canvas.get_allocation();
    cr->set_source_rgb( 0, 0, 0 );
    cr->rectangle( 0, 0, allocation.get_width(), allocation.get_height() );
    cr->fill();
  }   
  return false;
}
   
void GUI::onSeek() {
  gst_element_seek_simple( pipeline,
                           GST_FORMAT_TIME,
                           (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                           (gint64)(slider.get_value() * GST_SECOND)
  );
}
   
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

  gst_init (&argc, &argv);

  pipeline = gst_element_factory_make("playbin", "playbin");

  if(!pipeline)
    throw Exception() << __PRETTY_FUNCTION__ << ": Could not create playbin pipeline" << endl;

  g_object_set(pipeline, "uri", uri.c_str(), NULL);
   
  g_signal_connect( G_OBJECT( pipeline ), "video-tags-changed", (GCallback)&onMetadata, this);
  g_signal_connect( G_OBJECT( pipeline ), "audio-tags-changed", (GCallback)&onMetadata, this);
  g_signal_connect( G_OBJECT( pipeline ), "text-tags-changed" , (GCallback)&onMetadata, this);
   
  GstBus* bus = gst_element_get_bus( pipeline );
  gst_bus_add_signal_watch( bus );
  g_signal_connect( G_OBJECT( bus ), "message::error"        , (GCallback)&onError      , this );
  g_signal_connect( G_OBJECT( bus ), "message::eos"          , (GCallback)&onEndOfStream, this );
  g_signal_connect( G_OBJECT( bus ), "message::state-changed", (GCallback)&onStateChange, this );
  g_signal_connect( G_OBJECT( bus ), "message::application"  , (GCallback)&onNotify     , this );
  gst_object_unref( bus );
   
  if ( gst_element_set_state( pipeline, GST_STATE_PLAYING ) == GST_STATE_CHANGE_FAILURE )
    throw Exception() << __PRETTY_FUNCTION__ << ": Unable to set the pipeline to the playing state." << endl;
   
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
   
bool GUI::onTimeout(){
  GstFormat fmt = GST_FORMAT_TIME;
   
  if (pipelineState < GST_STATE_PAUSED)
    return true;
   
  if (!GST_CLOCK_TIME_IS_VALID (duration)) {
    if (!gst_element_query_duration (pipeline, fmt, &duration)) {
      cerr <<  "Could not query current duration." << endl;
    } else {
      slider.set_range(0, (double)duration / GST_SECOND);
    }
  }

  gint64 current = -1;
  if (gst_element_query_position (pipeline, fmt, &current)) {
    sliderConnection.block();
    slider.set_value((double)current / GST_SECOND);
    sliderConnection.unblock();
  }
  return true;
}
   
void GUI::onMetadata(GstBus* bus, GstMessage* stream, GUI *gui) {
  gst_element_post_message (gui->pipeline,
    gst_message_new_application (GST_OBJECT (gui->pipeline),
      gst_structure_new ("tags-changed", NULL)));
}
   
void GUI::onError (GstBus *bus, GstMessage *msg, GUI *gui) {
  GError *err;
  gchar *debug_info;
   
  gst_message_parse_error (msg, &err, &debug_info);
  cerr << "Error received from element " << GST_OBJECT_NAME (msg->src) << ":" << err->message << endl;
  cerr << "Debugging information: " << (debug_info ? debug_info : "none") << endl;
  g_clear_error (&err);
  g_free (debug_info);
   
  gst_element_set_state (gui->pipeline, GST_STATE_READY);
}
   
void GUI::onEndOfStream(GstBus *bus, GstMessage *msg, GUI *gui) {
  cout << "End-Of-Stream reached." << endl;
  gst_element_set_state (gui->pipeline, GST_STATE_READY);
}
   
void GUI::onStateChange(GstBus *bus, GstMessage *msg, GUI *gui) {
  GstState oldS, newS, pendingS;
  gst_message_parse_state_changed (msg, &oldS, &newS, &pendingS);
  if (GST_MESSAGE_SRC (msg) == GST_OBJECT (gui->pipeline)) {
    gui->pipelineState = newS;
    cout << "State set to " << gst_element_state_get_name (newS) << endl;
    if (oldS == GST_STATE_READY && newS == GST_STATE_PAUSED)
      gui->onTimeout();
  }
}
   
void GUI::onNotify(GstBus *bus, GstMessage *msg, GUI *gui) {
  if (string("tags-changed") == gst_structure_get_name(gst_message_get_structure(msg))) {
    ostringstream o;
    gint n_video, n_audio, n_text;
        
    g_object_get (gui->pipeline, "n-video", &n_video, NULL);
    g_object_get (gui->pipeline, "n-audio", &n_audio, NULL);
    g_object_get (gui->pipeline, "n-text", &n_text, NULL);
     
    for (int i = 0; i < n_video; i++) {
      GstTagList *tags = NULL;
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

GUI::~GUI(){
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
}
