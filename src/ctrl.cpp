
#include <CommReceiver.h>
#include <Motor.h>
#include <Servo.h>

#include <chrono>
#include <iostream>
#include <cstdint>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using std::cout;
using std::cerr;
using std::endl;
using namespace std::chrono;
using namespace boost::program_options;
using namespace boost::filesystem;

int main(int argc, char** argv){
  string progName=path(argv[0]).filename().native();
  path defaultConfigFile = current_path();
  defaultConfigFile/="cfg";
  defaultConfigFile/=progName+".cfg";

  options_description options(progName);
  options.add_options()
    ("configFile",         value<path>()->default_value(defaultConfigFile), 
                                                    "Path to configuration file")
    ("Connection.port",    value<unsigned short>(), "Port number")
    ("Connection.timeout", value<unsigned long>(),  "Timeout of periodic transmission in milliseconds")
    ("LEDs.maxBrightness", value<unsigned int>(),   "Maximum brightness value of leds")
    ("LEDs.number",        value<uint8_t>,          "Number of present leds")
    ("Motor.m",            value<int>(),            "Linear factor of motor calibration")
    ("Motor.n",            value<unsigned int>(),   "Linear offset of motor calibration")
    ("Motor.max",          value<int>(),            "Maximum speed value of motor")
    ("Motor.min",          value<int>(),            "Minimum speed value of motor")
    ("Motor.frequency",    value<unsigned int>(),   "PWM frequency of motor signal")
    ("Motor.bits",         value<uint8_t>(),        "Number of bits of PWM signal")
    ("Motor.pin",          value<uint8_t>(),            "GPIO pin to output motor signal")


    ("Servo.m",            value<int>(),            "Linear factor of servo calibration")
    ("Servo.n",            value<int>(),            "Linear offset of servo calibration")
    ("Servo.maxAngle",     value<unsigned int>(),   "Maximum angle value of servo")
    ("help",                                        "print this help");
  variables_map vm;
  store(parse_command_line(argc, argv, options), vm);
  if(vm.count("help")){
    cerr << options << endl;
    return -1;
  }
  cout << "Mobile PI On-Board Control Application" << endl;
  ifstream configFile(vm["configFile"].as<path>());
  cout << "\tConfig file: " << vm["configFile"].as<path>() << endl;
  store(parse_config_file(configFile, options), vm);

  Motor::Config motorCfg;
  motorCfg.n=vm["Motor.n"].as<unsigned int>();
  motorCfg.m=vm["Motor.m"].as<int>();
  motorCfg.max=vm["Motor.max"].as<int>();
  motorCfg.min=vm["Motor.min"].as<int>();
  motorCfg.frequency=vm["Motor.frequency"].as<unsigned int>();
  motorCfg.pin=vm["Motor.pin"].as<uint8_t>();
  motorCfg.bits=vm["Motor.bits"].as<uint8_t>();
  Motor motor(motorCfg);
  Servo servo(vm["Servo.maxAngle"].as<unsigned int>(),
              vm["Servo.n"].as<unsigned int>()
              vm["Servo.m"].as<int>());
  CommReceiver remote(vm["Connection.port"].as<uint16_t>(), 
                      milliseconds(vm["Connection.timeout"].as<unsigned long>()),
                      vm["LEDs.number"].as<uint8_t>(),
                      vm["LEDs.maxBrightness"].as<unsigned int>(),
                      motor.config().max,
                      servo.config().max);

  cout << "\t" << connection << endl;
  cout << "\t" << parameters << endl;
  cout << "\t" << motor << endl;
  cout << "\t" << servo << endl;

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
  return 0;
}
