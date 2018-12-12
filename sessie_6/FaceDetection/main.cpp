#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?  |      | print this message   }"
        "{faces_vid p1    |      | <required> path to a video with faces   }"
        "{haarcascade p2  |      | <required> path to the haarcascade facedetection xml  }"
        "{lbpcascade p3   |      | <required> path to the lbpcascade facedetection xml  }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string faces_vid_loc(parser.get<string>("faces_vid"));
    string people_vid_loc(parser.get<string>("people_vid"));
    string haarcascade_loc(parser.get<string>("haarcascade"));
    string lbpcascade_loc(parser.get<string>("lbpcascade"));
    //Check of de argumenten niet leeg zijn
    if ( faces_vid_loc.empty() || people_vid_loc.empty() || haarcascade_loc.empty() || lbpcascade_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }


    ///6.1, 6.2: FACE DETECTION USING VIOLA-JONES HAAR MODEL & LBP MODEL

    //https://docs.opencv.org/3.0-beta/modules/videoio/doc/reading_and_writing_video.html
    VideoCapture cap(faces_vid_loc); // open the video
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat frame;
    namedWindow("frames", WINDOW_AUTOSIZE);

    CascadeClassifier detectorHAAR(haarcascade_loc);
    CascadeClassifier detectorLBP(lbpcascade_loc);

    while(cap.read(frame))
    {
        Mat frameHAAR = frame.clone();
        Mat frameLBP = frame.clone();
        vector<Rect> objectsHAAR, objectsLBP;
        vector<int> scoreHAAR, scoreLBP;

        //Detect the faces
        detectorHAAR.detectMultiScale(frameHAAR, objectsHAAR, scoreHAAR, 1.05, 3);
        detectorLBP.detectMultiScale(frameLBP, objectsLBP, scoreLBP, 1.05, 3);

        //Plot rects
        Mat canvas = frame.clone();
        for( int i = 0; i < objectsHAAR.size(); i++ )
        {
            //Determine the center of the circle, the radius, the text position and the colour
            int centerX = objectsHAAR[i].x + objectsHAAR[i].width/2;
            int centerY = objectsHAAR[i].y + objectsHAAR[i].height/2;
            int radius = (objectsHAAR[i].width + objectsHAAR[i].height)/2;
            int textX = objectsHAAR[i].x;
            int textY = objectsHAAR[i].y;
            Scalar colour = Scalar(255, 255, 0);

            circle(canvas, Point(centerX, centerY), radius, colour, 1);
            stringstream temp;
            temp << (int)scoreHAAR[i];
            putText(canvas, temp.str(), Point(textX, textY), FONT_HERSHEY_SIMPLEX, 1, colour);
        }

        for( size_t i = 0; i < objectsLBP.size(); i++ )
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
