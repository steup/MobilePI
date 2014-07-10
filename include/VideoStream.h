#pragma once

#include <string>
#include <cstdint>

#include <gst/gst.h>

namespace Gtk{
  class Widget;
}

class VideoStream{
  public:
    enum States{
      play = GST_STATE_PLAYING,
      pause = GST_STATE_PAUSED,
      stop = GST_STATE_READY
    };
  private:

    States      mState    = stop;
    int64_t     mDuration = GST_CLOCK_TIME_NONE;
    std::string mInfo;
    GstElement* mPipeline;

    static void onMetadata   ( GstBus* bus, GstMessage* msg, VideoStream* stream );
    static void onStateChange( GstBus* bus, GstMessage* msg, VideoStream* stream );
    static void onEndOfStream( GstBus *bus, GstMessage *msg, VideoStream *stream ) { stream->state(stop); }
    static void onError      ( GstBus* bus, GstMessage* msg, VideoStream* stream );

  public:
    VideoStream(int argc, char** argv);
    ~VideoStream();
    void widget(Gtk::Widget& widget);
    void uri(const std::string& uri) { g_object_set(mPipeline, "uri", uri.c_str(), NULL); }
    void state(States newState)      { gst_element_set_state (mPipeline, (GstState)newState); }
    States state() const             { return mState; }
    const std::string& info() const  { return mInfo; }
    double duration() const;
    double position() const;
    void position(double newPos);
};
