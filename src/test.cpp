#include <VideoGUI.h>
#include <Exception.h>

#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  try{  
    if(argc-- <=1)
      throw Exception() << "usage: " << argv[0] << " <file>" << endl;
    
    VideoGUI gui(argc, argv, "glade/video.glade");

    cout << "playing file: " << argv[1] << endl;
    gui.file(argv[1]);

    gui.run();
  }
  catch(const exception& e){
    cerr << "Exception: " << e.what() << endl;
    return -1;
  }

  return 0;
}
