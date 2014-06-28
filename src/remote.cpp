#include <CommSender.h>
#include <Joystick.h>
#include <Log.h>

#include <chrono>
#include <iostream>
#include <string>

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

int main(int argc, char** argv){
  try{
    string progName=path(argv[0]).filename().native();
    path defaultConfigFile = current_path();
    defaultConfigFile/="cfg";
    defaultConfigFile/=progName+".cfg";

    options_description options(progName);
    options.add_options()
      ("configFile", value<path>()->default_value(defaultConfigFile), "Path to configuration file")
      ("Log.file", value<string>(), "File to log errors or - for stdout")
      ("Connection.host", value<string>(), "Name of host to control")
      ("Connection.port", value<uint16_t>(), "Port of host to control")
      ("Connection.timeout", value<unsigned long>(), "Timeout of periodic transmission in milliseconds")
      ("Joystick.id", value<unsigned int>(), "Id of joystick to use")
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

    Joystick   joystick  (vm["Joystick.id"]       .as<unsigned int>());
    CommSender connection(vm["Connection.host"]   .as<string>(),
                          vm["Connection.port"]   .as<uint16_t>(),
                          milliseconds(vm["Connection.timeout"].as<unsigned long>()));
    const auto& parameters=connection.getParameters();

    cout << "\t" << connection << endl;
    cout << "\t" << joystick   << endl;
    cout << "\t" << parameters << endl;

    auto joystickHandler = [&](const Joystick::State& state){
      cout << state.axes[3] << ", " << state.axes[0] << endl;
      int16_t speed=-(int32_t)state.axes[3]*parameters.maxSpeed/state.axisMax;
      int16_t angle=(int32_t)state.axes[0]*parameters.maxAngle/state.axisMax;
      cout << speed << ", " << angle << endl;
      connection.setMoveData({speed, angle});
    };

    auto errorHandler = [&](std::exception& e)throw(){
      errorLog << e.what() << std::endl;
    };

    joystick.addEventHandler(joystickHandler);
    joystick.addErrorHandler(errorHandler);
    connection.addErrorHandler(errorHandler);

    pause();
  }
  catch(std::exception& e){
    cerr << std::endl;
    cerr << "Error during startup of program:" << std::endl;
    cerr << "\t" << e.what() << std::endl;
  }

  return 0;
}
