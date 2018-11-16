#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?   |      | print this message   }"
        "{image_search p1  |      | <required> path to image in which the template needs to be found   }"
        "{image_template p2|      | <required> path to image of the template  }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string image_search_loc(parser.get<string>("image_search"));
    string image_template_loc(parser.get<string>("image_template"));
    //Check of de argumenten niet leeg zijn
    if ( image_search_loc.empty() || image_template_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }
}
