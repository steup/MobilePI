#include <VideoStream.h>

#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  try{
    GUI gui(argc, argv);
    gui.run();
  }
  catch(const exception& e){
    cerr << "Exception: " << e.what() << endl;
    return -1;
  }

  return 0;
}
