#include <VideoStream.h>
#include <Exception.h>

#include <string>
#include <sstream>

#include <gtkmm/widget.h>
#include <gdk/gdkx.h>
#include <gst/video/videooverlay.h>
#include <iostream>

using namespace std;

static const char* sourceType   = "videotestsrc";
static const char* sinkType     = "xvimagesink";

VideoStream::VideoStream(int argc, char** argv, Gtk::Widget& widget) : mTarget(widget) {
  
  char* arg=new char[128];
  strcpy(arg, "--debug_level=9");
  char** myArgv = new char*[2];
  myArgv[0] = argv[0];
  myArgv[1] = arg;
  int myArgc=1;

  gst_init (&myArgc, &myArgv);
  delete[] arg;
  delete[] myArgv;

  mSource = gst_element_factory_make (sourceType, "source");
  if(!mSource)
    throw Exception() << __PRETTY_FUNCTION__ << ": Pipeline source " << sourceType << " could not be created";

  mSink = gst_element_factory_make (sinkType, "sink");
  if(!mSink)
    throw Exception() << __PRETTY_FUNCTION__ << ": Pipeline sink " << sinkType << " could not be created";
  
  g_object_set( G_OBJECT(mSink), "autopaint-colorkey", FALSE, NULL);

  mPipeline = gst_pipeline_new ("pipeline");
    
  if (!mPipeline)
    throw Exception() << __PRETTY_FUNCTION__ << ": Pipeline  could not be created";
   
  gst_bin_add_many (GST_BIN (mPipeline), mSource, mSink, NULL);
  if (gst_element_link( mSource, mSink ) == false )
    throw Exception() << __PRETTY_FUNCTION__ << ": Cannot link source and decoder";

  GstBus* bus = gst_element_get_bus( mPipeline );
  gst_bus_add_signal_watch( bus );
  g_signal_connect( G_OBJECT( bus ), "message::error"        , (GCallback)&onError      , this );
  g_signal_connect( G_OBJECT( bus ), "message::eos"          , (GCallback)&onEndOfStream, this );
  g_signal_connect( G_OBJECT( bus ), "message::state-changed", (GCallback)&onStateChange, this );
  gst_bus_set_sync_handler( bus, (GstBusSyncHandler)&onPreparation, this, NULL);
  gst_object_unref( bus );
}

VideoStream::~VideoStream(){
  gst_element_set_state (mPipeline, GST_STATE_NULL);
  gst_object_unref (mPipeline);
}

GstBusSyncReply VideoStream::onPreparation(GstBus* bus, GstMessage* msg, VideoStream* stream) {
  if (!gst_is_video_overlay_prepare_window_handle_message (msg))
   return GST_BUS_PASS;
   
   auto nativeWindow = stream->mTarget.get_window();
   
  if (!nativeWindow->ensure_native())
    throw Exception() << __PRETTY_FUNCTION__ << ": Window " << &stream->mTarget << " not native. Cannot draw video to it!" << endl;
   
  gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY (stream->mSink), GDK_WINDOW_XID(nativeWindow->gobj()));
  /** \bug the window is not drawn, without the call to get_geometry, might be a X window, GDK bug or a race condition*/
  int x,y,w,h;
  nativeWindow->get_geometry(x,y,w,h);
  /*gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY (mSink), x, y, w, h);
  cout << "(x,y,w,h) = (" << x << ", " << y << ", " << ", " << w << ", " << h << ")" << endl;*/
  return GST_BUS_DROP;
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

void VideoStream::state(VideoStream::States newState) {
  gst_element_set_state (mPipeline, (GstState)newState);
}
