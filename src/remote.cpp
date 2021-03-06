#include <CommSender.h>
#include <Joystick.h>
#include <Log.h>
#include <GUI.h>

#include <memory>
#include <chrono>
#include <iostream>
#include <string>

#include <signal.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/exception/diagnostic_information.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using namespace std::chrono;
using namespace boost::program_options;
using namespace boost::filesystem;

GUI* guiPtr = nullptr;

void stop(int sig){
  if(guiPtr)
    guiPtr->stop();
}

int main(int argc, char** argv){
  signal(SIGINT, stop);
  signal(SIGTERM, stop);
  signal(SIGQUIT, stop);
  signal(SIGHUP, stop);
  options_description options;

  try{   
    string progName=path(argv[0]).filename().native();
    path defaultConfigFile = current_path();
    defaultConfigFile/="cfg";
    defaultConfigFile/=progName+".cfg";

    options.add_options()
      ("configFile", value<path>()->default_value(defaultConfigFile), "Path to configuration file")
      ("Log.file", value<string>(), "File to log errors or - for stdout")
      ("Connection.host", value<string>(), "Name of host to control")
      ("Connection.port", value<uint16_t>(), "Port of host to control")
      ("Connection.timeout", value<unsigned long>(), "Timeout of periodic transmission in milliseconds")
      ("Joystick.id", value<unsigned int>(), "Id of joystick to use")
      ("GUI.gladeFile", value<string>(), "Glade file describing used GUI elements")
      ("GUI.updateRate", value<uint16_t>(), "Update rate of GUI in fps")
      ("listJoysticks", "List available joysticks by ID")
      ("help", "print this help");

    variables_map vm;
    store(parse_command_line(argc, argv, options), vm);

    if(vm.count("listJoysticks")){
      cout << "Available Joysticks: " << endl;
      for(unsigned int i=0;i<Joystick::getNumJoysticks();i++)
        cout << "\t" << i << " " << Joystick::getAvailableJoysticks().at(i) << endl;
      return 0;
    }
    if(vm.count("help")){
      cerr << options << endl;
      return -1;
    }

    cout << "Mobile PI Remote Control Application:" << endl;
    ifstream configFile(vm["configFile"].as<path>());
    cout << "\tConfig file: " << vm["configFile"].as<path>() << endl;
    store(parse_config_file(configFile, options), vm);

    Log errorLog(vm["Log.file"].as<string>());



    GUI gui(vm["GUI.gladeFile"].as<string>(),
            vm["GUI.updateRate"].as<uint16_t>());
    guiPtr = &gui;

    Joystick   joystick  (vm["Joystick.id"]       .as<unsigned int>());
    CommSender connection(vm["Connection.host"]   .as<string>(),
                          vm["Connection.port"]   .as<uint16_t>(),
                          milliseconds(vm["Connection.timeout"].as<unsigned long>()));
    const auto& parameters=connection.getParameters();

    cout << "\t" << connection << endl;
    cout << "\t" << joystick   << endl;
    cout << "\t" << parameters << endl;
    gui.joystick(joystick.name());

    auto joystickHandler = [&](const Joystick::State& state){
      int16_t speed=-(int32_t)state.axes[3]*parameters.maxSpeed/state.axisMax;
      int16_t angle=(int32_t)state.axes[0]*parameters.maxAngle/state.axisMax;
      connection.setMoveData({speed, angle});
      gui.value({(float)speed/parameters.maxSpeed, (float)angle/parameters.maxAngle});
    };

    auto errorHandler = [&](const std::exception& e)throw(){
      errorLog << e.what() << std::endl;
    };

    joystick.addEventHandler(joystickHandler);
    joystick.addErrorHandler(errorHandler);
    connection.addErrorHandler(errorHandler);

    try{
      return gui.run();
    }
    catch(const Glib::Exception& e){
      errorLog << e.what() << std::endl;
    }
  }
  catch(const boost::bad_any_cast& e){
    cerr << endl;
    cerr << "Invalid Command Line or Config file:" << endl;
    cerr << "\t" << e.what() << endl;
    cerr << endl << "Available options:" << endl << options << endl;
  }
  catch(const std::exception& e){
    cerr << endl;
    cerr << "Error during startup of program:" << endl;
    cerr << "\t" << e.what() << endl;
  }
  catch(const Glib::Exception& e){
    cerr << endl;
    cerr << "Error during startup of program:" << endl;
    cerr << "\t" << e.what() << endl;
  }

  return -1;
}
