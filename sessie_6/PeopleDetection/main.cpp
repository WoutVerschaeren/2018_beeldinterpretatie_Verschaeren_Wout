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
        "{haarcascade p3  |      | <required> path to the haarcascade facedetection xml  }"
        "{lbpcascade p4   |      | <required> path to the lbpcascade facedetection xml  }"
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
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

    while(cap.read(frame))
    {
        Mat frameHOG = frame.clone();
        vector<Rect> objectsHOG;

        //Detect the faces
        hog.detectMultiScale(frameHOG, objectsHOG, 0, Size(8,8), Size(32,32), 1.05, 2);

        //Plot rects
        Mat canvas = frame.clone();
        for( int i = 0; i < objectsHOG.size(); i++ )
        {
            //Determine the center of the circle, the radius, the text position and the colour
            int centerX = objectsHOG[i].x + objectsHOG[i].width/2;
            int centerY = objectsHOG[i].y + objectsHOG[i].height/2;
            int radius = (objectsHOG[i].width + objectsHOG[i].height)/2;
            Scalar colour = Scalar(255, 255, 0);

            circle(canvas, Point(centerX, centerY), radius, colour, 1);
        }

        /*for( size_t i = 0; i < objectsLBP.size(); i++ )
        {
            //Determine the center of the circle, the radius, the text position and the colour
            int centerX = objectsLBP[i].x + objectsLBP[i].width/2;
            int centerY = objectsLBP[i].y + objectsLBP[i].height/2;
            int radius = (objectsLBP[i].width + objectsLBP[i].height)/2;
            int textX = objectsLBP[i].x + objectsLBP[i].width;
            int textY = objectsLBP[i].y + objectsLBP[i].height;
            Scalar colour = Scalar(0, 0, 255);

            circle(canvas, Point(centerX, centerY), radius, colour, 1);
            stringstream temp;
            temp << (int)scoreLBP[i];
            putText(canvas, temp.str(), Point(textX, textY), FONT_HERSHEY_SIMPLEX, 1, colour);
        }*/

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
