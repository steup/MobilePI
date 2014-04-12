#include <CommSender.h>
#include <Joystick.h>

#include <chrono>
#include <thread>
#include <iostream>
#include <limits>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>


using namespace std;
using namespace chrono;
using namespace this_thread;
using namespace boost::program_options;

string progName(const char* arg){
  string progName=arg;
  size_t n=progName.find_last_of("/");
  if(n==string::npos)
    n=0;
  return progName.substr(n+1);
}

void startSender(std::string host, uint16_t port, unsigned int joyDev){
  cout << "Mobile PI Remote Control Application " << endl;

  CommSender remote(host, port, 500);

  Joystick joy(joyDev);

  auto joyHandler = [&](const Joystick::State& state){
    cout << state.axes[3] << ", " << state.axes[0] << endl;
    int16_t speed=(int32_t)state.axes[3]*-1*remote.getParameters().maxSpeed/numeric_limits<int16_t>::max();
    int16_t angle=(int32_t)state.axes[0]*remote.getParameters().maxAngle/numeric_limits<int16_t>::max();
    cout << speed << ", " << angle << endl;
    remote.setMoveData({speed, angle});
  };

  joy.addEventHandler(joyHandler);

  pause();
  exit(0);
}

int main(int argc, char** argv){
  stringstream ss;
  ss << progName(argv[0]) << " (options) <port>";

  options_description options(ss.str());
  options.add_options()
    ("host", value<string>(), "Target host")
    ("port", value<uint16_t>(), "Port number ")
    ("joyDev", value<unsigned int>(), "ID of joystick to use")
    ("listJoysticks", "List available joysticks by ID")
    ("help", "print this help");
  positional_options_description pOpts;
  pOpts.add("host", 1);
  pOpts.add("port", 1);
  pOpts.add("joyDev", 1);
  variables_map vm;
  store(command_line_parser(argc, argv).options(options).positional(pOpts).run(), vm);
  if(vm.count("listJoysticks")){
    cout << "Available Joysticks: " << endl;
    for(unsigned int i=0;i<Joystick::getNumJoysticks();i++)
      cout << "\t" << i << Joystick::getAvailableJoysticks().at(i) << endl;
    return 0;
  }
  if(!vm.count("port") || !vm.count("host") || !vm.count("joyDev") || vm.count("help")){
    cerr << options << endl;
    return -1;
  }

  if(vm["joyDev"].as<unsigned int>() >= Joystick::getNumJoysticks()){
    cerr << "Error: Joystick " << vm["joyDev"].as<unsigned int>() << " not available!" << endl;
    return -1;
  }

  startSender(vm["host"].as<string>(), vm["port"].as<uint16_t>(), vm["joyDev"].as<unsigned int>());
  return 0;
}
