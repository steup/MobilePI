#include <HelloWorld.h>

#include <gtkmm/application.h>

#include <iostream>

int main(int argc, char** argv){
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv);
  Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();

  builder->add_from_file("glade/HelloWorld.glade");

  HelloWorld* window = NULL;

  builder->get_widget_derived("Window", window);
  
  if(window)
    return app->run(*window);
  else{
    std::cerr << "No such widget " << "Window" << std::endl;
    return -1;
  }
}
