#include <VideoGUI.h>
#include <Exception.h>

#include <boost/filesystem.hpp>

#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/textview.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>

#include <glibmm/main.h>

using namespace boost::filesystem;
using namespace std;
   
bool VideoGUI::onDraw(const Cairo::RefPtr<Cairo::Context>& cr) {
  if (stream.state() != VideoStream::play) {
    auto allocation = canvas.get_allocation();
    cr->set_source_rgb( 0, 0, 0 );
    cr->rectangle( 0, 0, allocation.get_width(), allocation.get_height() );
    cr->fill();
  }   
  return false;
}

template<typename T>
T& VideoGUI::loadWidget(const std::string& name){
  T* widgetPtr = nullptr;
  builder->get_widget(name, widgetPtr);
  if(widgetPtr)
    return *widgetPtr;
  else
    throw Exception() << __PRETTY_FUNCTION__ << ": " << __LINE__ << " - Unable to find widget " << name << "!" << endl;
}

   
VideoGUI::VideoGUI(int argc, char** argv, const string& gladeFile)
  : stream(argc, argv),
    app(Gtk::Application::create(argc, argv)),
    builder(Gtk::Builder::create_from_file(gladeFile)),
    window(loadWidget<Gtk::Window>("Window")),
    canvas(loadWidget<Gtk::DrawingArea>("Video")),
    seek(loadWidget<Gtk::Scale>("Seek")),
    info(loadWidget<Gtk::TextView>("Info")){

  Glib::signal_timeout().connect(sigc::mem_fun(*this, &VideoGUI::onTimeout), 1000);

  canvas.signal_realize().connect(sigc::mem_fun(*this, &VideoGUI::onRealize));
  canvas.signal_draw().connect(sigc::mem_fun(*this, &VideoGUI::onDraw));
  
  loadWidget<Gtk::Button>("Play").signal_clicked().connect(sigc::mem_fun(*this, &VideoGUI::onPlay));
  loadWidget<Gtk::Button>("Pause").signal_clicked().connect(sigc::mem_fun(*this, &VideoGUI::onPause));
  loadWidget<Gtk::Button>("Stop").signal_clicked().connect(sigc::mem_fun(*this, &VideoGUI::onStop));

  seekConnection = seek.signal_value_changed().connect(sigc::mem_fun(*this, &VideoGUI::onSeek));
}

void VideoGUI::file(const std::string& file){
  auto fileName = canonical(file);
  uri = "file://";
  uri += fileName.native();
}

void VideoGUI::run(){

  window.show_all();

  if(!uri.empty()){
    stream.uri(uri);
    stream.state(VideoStream::play);
  }
  app->run(window);
}
   
bool VideoGUI::onTimeout(){
  if(stream.state() != VideoStream::play)
    return true;

  seekConnection.block();
  auto position = stream.position();
  auto duration = stream.duration();
  if(duration>0)
    seek.set_range(0, duration);
  if(position>0)
    seek.set_value(position);
  seekConnection.unblock();

  info.get_buffer()->set_text(stream.info());

  return true;
}
