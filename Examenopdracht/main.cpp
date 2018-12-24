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

    //https://docs.opencv.org/3.0-beta/modules/videoio/doc/reading_and_writing_video.html
    VideoCapture cap(mario_vid_loc); // open the video
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat frame;
    namedWindow("frames", WINDOW_AUTOSIZE);

    while(cap.read(frame))
    {
        imshow("Frames (press space to pause, any other key to quit)", frame);
        //Press space to pause, any other key to exit
        int k = waitKey(25);
        if ( k == 32 )
            waitKey(0);
        else if ( k >= 0 )
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
