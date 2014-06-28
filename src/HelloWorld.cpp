#include <HelloWorld.h>

#include <gtkmm/button.h>
#include <glibmm/main.h>

#include <iostream>
#include <sstream>
#include <chrono>

HelloWorld::HelloWorld(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
  : Gtk::Window(cobject){
  Gtk::Button* button=NULL;
  builder->get_widget("Button", button);
  builder->get_widget("Counter", mCounterLabel);
  builder->get_widget("Time", mTimeLabel);
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &HelloWorld::onTimeout), 1000/1);

  if(button)
    button->signal_clicked().connect(sigc::mem_fun(*this, &HelloWorld::onClick));
}

HelloWorld::~HelloWorld(){}

void HelloWorld::onClick(){
  std::ostringstream out;
  out << ++mCounter;
  if(mCounterLabel)
    mCounterLabel->set_text(out.str());
  std::cout << "Hello World: " << mCounter << std::endl;
}

bool HelloWorld::onTimeout(){
  std::ostringstream oss;
  auto now = std::chrono::system_clock::now().time_since_epoch();
  oss << now.count();
  if(mTimeLabel)
    mTimeLabel->set_text(oss.str());
  std::cout << "The time: " << now.count() << std::endl;
  return true;
}
