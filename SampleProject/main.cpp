#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?  |      | print this message   }"
        "{image_colour p1 |      | <required> path to colour image   }"
        "{image_bimodal p2|      | <required> path to bimodal image  }"
        "{image_adapted p3|      | <required> path to adapted colour image   }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string image_colour_loc(parser.get<string>("image_colour"));
    string image_bimodal_loc(parser.get<string>("image_bimodal"));
    string image_adapted_loc(parser.get<string>("image_adapted"));
    //Check of de argumenten niet leeg zijn
    if ( image_colour_loc.empty() || image_bimodal_loc.empty() || image_adapted_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }
}
