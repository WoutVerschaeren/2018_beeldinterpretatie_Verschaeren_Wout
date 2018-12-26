#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int slider_max = 100;
int thr;                        //max threshold value
int alpha_slider_thr = 20;      //slider for threshold value


static void thr_on_trackbar(int, void *)
{
    thr = alpha_slider_thr;
}

///Finds a match in the source image src using the template tmpl
Mat matchSingle(Mat img, Mat tmpl)
{
    Mat result;

    ///Do the matching and normalize
    matchTemplate(img.clone(), tmpl, result, TM_SQDIFF_NORMED);
    //SQDIFF represents the best hits with the lowest values: invert!
    result = 1.0-result;
    //imshow("Match map single", result); waitKey(0);

    ///Find the best hit
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
    matchLoc = maxLoc;

    double thr_norm = (thr*1.0)/100;

    if ( maxVal >= thr_norm )
    {
        //Accept the match
        Mat best_hit = img.clone();
        rectangle(best_hit, matchLoc, Point(matchLoc.x + tmpl.cols , matchLoc.y + tmpl.rows), Scalar::all(255), 2, 8, 0);

        return best_hit;
    }
    else
    {
        //Reject the match
        return img;
    }
}

void slider(Mat img, Mat templ)
{
    String imTitle = "Super Mario World";
    namedWindow(imTitle, WINDOW_AUTOSIZE); // Create Window
    //Create S slider
    createTrackbar("Mario threshold", imTitle, &alpha_slider_thr, slider_max, thr_on_trackbar);

    thr_on_trackbar(alpha_slider_thr, 0);

    while ( true )
    {
        Mat best_hit = matchSingle(img, templ);
        //Show the image with the mask applied
        imshow(imTitle, best_hit);

        int k = waitKey(10);
        if ( k == 32 ) //spacebar
        {
            destroyWindow(imTitle);
            break;
        }
    }
}

int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?  |      | print this message   }"
        "{mario_vid p1    |      | <required> path to a video of a Super Mario World level   }"
        "{mini_mario p2   |      | <required> path to a template of Mini Mario's head   }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string mario_vid_loc(parser.get<string>("mario_vid"));
    string mini_mario_loc(parser.get<string>("mini_mario"));
    //Check of de argumenten niet leeg zijn
    if ( mario_vid_loc.empty() || mini_mario_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }

    //int templ = 1;
    Mat minimario = imread(mini_mario_loc);

    //https://docs.opencv.org/3.0-beta/modules/videoio/doc/reading_and_writing_video.html
    VideoCapture cap(mario_vid_loc); // open the video
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat frame;
    //namedWindow("Frames (press space to pause, any other key to quit)", WINDOW_AUTOSIZE);

    //int speed = 25;

    while(cap.read(frame))
    {
        Mat matched = matchSingle(frame, minimario);
        imshow("Frames (press space to pause, any other key to quit)", matched);
        //Press space to pause, any other key to exit
        int k = waitKey(10);
        if ( k == 32 )
        {
            slider(frame, minimario);
        }
        else if ( k >= 0 )
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
