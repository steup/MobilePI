
#include <CommReceiver.h>
#include <Motor.h>
#include <Servo.h>

#include <chrono>
#include <iostream>
#include <cstdint>
#include <string>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
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
    ("LEDs.number",        value<uint8_t>(),        "Number of present leds")

    ("Motor.m",            value<int>(),            "Linear factor of motor calibration")
    ("Motor.n",            value<unsigned int>(),   "Linear offset of motor calibration")
    ("Motor.max",          value<int>(),            "Maximum speed value of motor")
    ("Motor.min",          value<int>(),            "Minimum speed value of motor")
    ("Motor.frequency",    value<unsigned int>(),   "PWM frequency of motor signal")
    ("Motor.bits",         value<uint16_t>(),        "Number of bits of pwm signal")
    ("Motor.pin",          value<uint16_t>(),        "GPIO pin to output motor signal")


    ("Servo.m",            value<int>(),            "Linear factor of servo calibration")
    ("Servo.n",            value<unsigned int>(),   "Linear offset of servo calibration")
    ("Servo.max",          value<int>(),            "Maximum angle value of servo")
    ("Servo.min",          value<int>(),            "Minimum angle value of servo")
    ("Servo.divider",      value<unsigned int>(),   "CPU frequency divider to create pwm frequency")
    ("Servo.bits",         value<uint16_t>(),        "Number of bits in pwm signal")

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
  motorCfg.pin=vm["Motor.pin"].as<uint16_t>();
  motorCfg.bits=vm["Motor.bits"].as<uint16_t>();
  Motor motor(motorCfg);

  Servo::Config servoCfg;
  servoCfg.min=vm["Servo.min"].as<int>();
  servoCfg.max=vm["Servo.max"].as<int>();
  servoCfg.m=vm["Servo.m"].as<int>();
  servoCfg.n=vm["Servo.n"].as<unsigned int>();
  servoCfg.divider=vm["Servo.divider"].as<unsigned int>();
  servoCfg.bits=vm["Servo.bits"].as<uint16_t>();
  Servo servo(servoCfg);

  CommReceiver connection(vm["Connection.port"].as<uint16_t>(), 
                          milliseconds(vm["Connection.timeout"].as<unsigned long>()),
                          vm["LEDs.number"].as<uint8_t>(),
                          vm["LEDs.maxBrightness"].as<unsigned int>(),
                          motor.config().max,
                          servo.config().max);

  cout << "\t" << connection << endl;
  cout << "\t" << connection.getParameters() << endl;
  cout << "\t" << motor.config() << endl;
  cout << "\t" << servo.config() << endl;

  auto errorHandler = [&](CommError e){
    cerr << e.what() << endl;
    motor.speed(0);
  };

  auto eventHandler = [&](){
    try{
      motor.speed(connection.getMoveData().speed);
      servo.angle(connection.getMoveData().angle);
    }
    catch(MotorException e){
      cerr << e.what() << endl;
    }
    catch(ServoException e){
      motor.speed(0);
      cerr << e.what() << endl;
    }
  };

  connection.addErrorHandler(errorHandler);
  connection.addEventHandler(eventHandler);

  pause();
  return 0;
}
