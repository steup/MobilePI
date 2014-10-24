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
    GstElement* mPipeline = nullptr;
    GstElement* mSink     = nullptr;
    GstElement* mSource   = nullptr;
    std::string mInfo     = "nothing yet";
    Gtk::Widget& mTarget;

    static void onStateChange( GstBus*     bus, GstMessage* msg, VideoStream* stream );
    static void onEndOfStream( GstBus*     bus, GstMessage *msg, VideoStream *stream ) { stream->state(stop); }
    static void onError      ( GstBus*     bus, GstMessage* msg, VideoStream* stream );
    static void onPadCreation( GstElement* bus, GstPad*     msg, VideoStream* stream );
    static GstBusSyncReply onPreparation( GstBus*     bus, GstMessage* msg, VideoStream* stream );

  public:
    VideoStream(int argc, char** argv, Gtk::Widget& widget);
    ~VideoStream();
    void state(States newState);
    const std::string& info() const  { return mInfo;  }
    States state() const             { return mState; }
};
