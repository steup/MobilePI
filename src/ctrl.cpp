
#include <CommReceiver.h>
#include <Motor.h>
#include <Servo.h>

#include <chrono>
#include <thread>
#include <iostream>

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

void startReceiver(uint16_t port){
  cout << "Mobile PI Control Application " << port << endl;

  Motor motor;
  Servo servo(300);
  CommReceiver remote(port, 1000, 4, 255, motor.config().max, servo.angle());

  auto errorHandler = [&](CommError e){
    cerr << e.what() << endl;
    motor.speed(0);
  };

  auto eventHandler = [&](){
    try{
      motor.speed(remote.getMoveData().speed);
      servo.angle(remote.getMoveData().angle);
    }
    catch(MotorException e){
      cerr << e.what() << endl;
    }
    catch(ServoException e){
      motor.speed(0);
      cerr << e.what() << endl;
    }
  };

  remote.addErrorHandler(errorHandler);
  remote.addEventHandler(eventHandler);

  pause();
}

int main(int argc, char** argv){
  stringstream ss;
  ss << progName(argv[0]) << " (options) <port>";

  options_description options(ss.str());
  options.add_options()
    ("port", value<unsigned short>(), "Port number")
    ("help", "print this help");
  positional_options_description pOpts;
  pOpts.add("port", 1);
  variables_map vm;
  store(command_line_parser(argc, argv).options(options).positional(pOpts).run(), vm);
  if(!vm.count("port")){
    cerr << options << endl;
    return -1;
  }

  startReceiver(vm["port"].as<uint16_t>());
  return 0;
}
