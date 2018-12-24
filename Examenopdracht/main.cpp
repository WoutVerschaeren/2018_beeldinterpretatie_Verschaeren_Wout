#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?  |      | print this message   }"
        "{mario_vid p1    |      | <required> path to a video of a Super Mario World level   }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string mario_vid_loc(parser.get<string>("mario_vid"));
    //Check of de argumenten niet leeg zijn
    if ( mario_vid_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }
}
