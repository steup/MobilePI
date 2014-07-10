#include <VideoStream.h>
#include <Exception.h>

#include <string>
#include <sstream>

#include <gtkmm/widget.h>
#include <gdk/gdkx.h>
#include <gst/video/videooverlay.h>

using namespace std;

VideoStream::VideoStream(int argc, char** argv){
  
  gst_init (&argc, &argv);

  mPipeline = gst_element_factory_make("playbin", "playbin");

  if(!mPipeline)
    throw Exception() << __PRETTY_FUNCTION__ << ": Could not create playbin mPipeline" << endl;
   
  g_signal_connect( G_OBJECT( mPipeline ), "video-tags-changed", (GCallback)&onMetadata, this);
  g_signal_connect( G_OBJECT( mPipeline ), "audio-tags-changed", (GCallback)&onMetadata, this);
  g_signal_connect( G_OBJECT( mPipeline ), "text-tags-changed" , (GCallback)&onMetadata, this);
   
  GstBus* bus = gst_element_get_bus( mPipeline );
  gst_bus_add_signal_watch( bus );
  g_signal_connect( G_OBJECT( bus ), "message::error"        , (GCallback)&onError      , this );
  g_signal_connect( G_OBJECT( bus ), "message::eos"          , (GCallback)&onEndOfStream, this );
  g_signal_connect( G_OBJECT( bus ), "message::state-changed", (GCallback)&onStateChange, this );
  gst_object_unref( bus );
}

VideoStream::~VideoStream(){
  gst_element_set_state (mPipeline, GST_STATE_NULL);
  gst_object_unref (mPipeline);
}

void VideoStream::widget(Gtk::Widget& widget) {
  auto nativeWindow = widget.get_window();
   
  if (!nativeWindow->ensure_native())
    throw Exception() << __PRETTY_FUNCTION__ << ": Window " << &widget << " not native. Cannot draw video to it!" << endl;
   
  gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY (mPipeline), GDK_WINDOW_XID(nativeWindow->gobj()));
}

double VideoStream::duration() const{
  int64_t value;
  if (gst_element_query_duration (mPipeline, GST_FORMAT_TIME, &value))
    return (double)value / GST_SECOND;
  else
    return -1;
}
   
void VideoStream::position(double newPos) {
  gst_element_seek_simple( mPipeline,
                           GST_FORMAT_TIME,
                           (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                           (int64_t)(newPos * GST_SECOND)
  );
}

double VideoStream::position() const {
  int64_t value;
  if(gst_element_query_position(mPipeline, GST_FORMAT_TIME, &value))
    return (double)value / GST_SECOND;
  else
    return 0;
}

void VideoStream::onError (GstBus *bus, GstMessage *msg, VideoStream *stream) {
  GError *err;
  gchar *debug_info;
  ostringstream o;
   
  gst_message_parse_error (msg, &err, &debug_info);
  o << "Error received from element " << GST_OBJECT_NAME (msg->src) << ":" << err->message << endl;
  o << "Debugging information: " << (debug_info ? debug_info : "none") << endl;
  g_clear_error (&err);
  g_free (debug_info);
  
  stream->mInfo = o.str();
  stream->state(stop);
}
   
void VideoStream::onStateChange(GstBus *bus, GstMessage *msg, VideoStream *stream) {
  GstState oldS, newS, pendingS;
  gst_message_parse_state_changed (msg, &oldS, &newS, &pendingS);
  if (GST_MESSAGE_SRC (msg) == GST_OBJECT (stream->mPipeline))
    stream->mState = (States)newS;
}
   
void VideoStream::onMetadata(GstBus *bus, GstMessage *msg, VideoStream *stream) {
  ostringstream o;
  gint n_video, n_audio, n_text;
      
  g_object_get (stream->mPipeline, "n-video", &n_video, NULL);
  g_object_get (stream->mPipeline, "n-audio", &n_audio, NULL);
  g_object_get (stream->mPipeline, "n-text", &n_text, NULL);
   
  for (int i = 0; i < n_video; i++) {
    GstTagList *tags = NULL;
    g_signal_emit_by_name (stream->mPipeline, "get-video-tags", i, &tags);
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
    g_signal_emit_by_name (stream->mPipeline, "get-audio-tags", i, &tags);
    if (tags) {
      char* str = NULL;
      unsigned int rate = 0;

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
    g_signal_emit_by_name (stream->mPipeline, "get-text-tags", i, &tags);
    if (tags) {
      char* str = NULL;
      o << endl << "subtitle stream " << i << ":" << endl;
      if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_CODE, &str)) {
        o << "  language: " << str << endl;
        g_free (str);
      }
      gst_tag_list_free (tags);
    }
  }
  stream->mInfo = o.str();
}
