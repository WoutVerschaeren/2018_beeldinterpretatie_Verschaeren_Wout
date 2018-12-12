#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?  |      | print this message   }"
        "{people_vid p1   |      | <required> path to a video of people walking  }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string people_vid_loc(parser.get<string>("people_vid"));
    //Check of de argumenten niet leeg zijn
    if ( people_vid_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }


    ///6.1, 6.2: FACE DETECTION USING VIOLA-JONES HAAR MODEL & LBP MODEL

    //https://docs.opencv.org/3.0-beta/modules/videoio/doc/reading_and_writing_video.html
    VideoCapture cap(people_vid_loc); // open the video
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat frame;
    namedWindow("frames", WINDOW_AUTOSIZE);

    HOGDescriptor hog;
    //Load standard people detector
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

    vector<Point> path;

    while(cap.read(frame))
    {
        Mat frameHOG = frame.clone();
        vector<Rect> objectsHOG;
        vector<double> foundWeights;
        double thresh = 0.2;

        //Detect the people
        hog.detectMultiScale(frameHOG, objectsHOG, foundWeights, thresh);

        //Plot rects
        Mat canvas = frame.clone();
        for( size_t i = 0; i < objectsHOG.size(); i++ )
        {
            //Draw a rectangle around the detected person
            Scalar colour = Scalar(0, 255, 0);
            rectangle(canvas, objectsHOG[i], colour, 1, 8, 0);

            int pathX = objectsHOG[i].x + objectsHOG[i].width/2;
            int pathY = objectsHOG[i].y + objectsHOG[i].height/2;

            path.push_back(Point(pathX, pathY));
        }

        Scalar colour = Scalar(255, 255, 255);
        for ( int j = 0; j < path.size(); j++ )
        {
            polylines(canvas, path, false, colour);
        }

        imshow("frames", canvas);
        //Press space to pause, any other key to exit
        int k = waitKey(1);
        if ( k == 32 )
            waitKey(0);
        else if ( k >= 0 )
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
