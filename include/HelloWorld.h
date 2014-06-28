#pragma once

#include <gtkmm/window.h>
#include <gtkmm/label.h>
#include <gtkmm/builder.h>

class HelloWorld : public Gtk::Window{
  private:
    Gtk::Label* mCounterLabel = NULL;
    Gtk::Label* mTimeLabel = NULL;
    unsigned int mCounter = 0;
  public:
    HelloWorld(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~HelloWorld();

  protected:
    void onClick();
    bool onTimeout();
};
