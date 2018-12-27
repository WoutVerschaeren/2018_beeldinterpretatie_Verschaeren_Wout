#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int AMOUNTTEMPL = 2;

const int slider_max = 100;
int thr;                        //max threshold value
int alpha_slider_thr = 35;      //slider for threshold value

vector<Point> path;
bool capHeight = 0;


static void thr_on_trackbar(int, void *)
{
    thr = alpha_slider_thr;
}

int getIndOfMax(array<double, AMOUNTTEMPL> maxVals)
{
    int maximum = 0;
    int indOfMax = -1;
    for ( size_t i = 0; i < maxVals.size(); i++ )
    {
        if ( maxVals[i] > maximum )
        {
            maximum = maxVals[i];
            indOfMax = i;
        }
    }

    return indOfMax;
}

///Finds a match in the source image src using the template tmpl
Mat match(Mat img, array<Mat, AMOUNTTEMPL> templates, double frameCount)
{
    Mat result;
    Mat canvas = img.clone();
    array<double, AMOUNTTEMPL> maxVals;
    array<Point, AMOUNTTEMPL> matchLocs;
    int indOfMax;

    for ( size_t i = 0; i < templates.size(); i++ ) {
        ///Do the matching
        matchTemplate(img.clone(), templates[i], result, TM_SQDIFF_NORMED);
        //SQDIFF represents the best hits with the lowest values: invert!
        result = 1.0-result;

        ///Find the best hit
        double minVal; double maxVal; Point minLoc; Point maxLoc;
        minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

        double thr_norm = (thr*1.0)/100;

        //Is the best hit sufficient?
        if ( maxVal >= thr_norm )
        {
            ///Accept the match
            //Remember the value and location of the best hit
            maxVals[i] = maxVal;
            matchLocs[i] = maxLoc;

            //Draw a rectangle around the found match
            rectangle(canvas, matchLocs[i], Point(matchLocs[i].x + templates[i].cols , matchLocs[i].y + templates[i].rows), Scalar::all(255), 2, 8, 0);

        }
        else
        {
            //Reject the match
            maxVals[i] = 0;
        }
    }

    indOfMax = getIndOfMax(maxVals);
    if ( indOfMax != -1 )
    {
        if ( capHeight )
        {
            path.push_back(Point(frameCount, matchLocs[indOfMax].y));
            cout << "Capping" << endl;
        }
    }


    return canvas;
}

void slider(Mat img, array<Mat, 2> templates, double frameCount)
{
    String imTitle = "Edit the threshold for detecting Mario";
    namedWindow(imTitle, WINDOW_AUTOSIZE); // Create Window
    //Create S slider
    createTrackbar("Mario threshold", imTitle, &alpha_slider_thr, slider_max, thr_on_trackbar);

    thr_on_trackbar(alpha_slider_thr, 0);

    while ( true )
    {
        Mat best_hit = match(img, templates, frameCount);
        //Show the image with the mask applied
        imshow(imTitle, best_hit);

        int k = waitKey(1);
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
        "{mario_vid       |      | <required> path to a video of a Super Mario World level   }"
        "{mini_marioR     |      | <required> path to a template of Mini Mario's head, facing right   }"
        "{mini_marioL     |      | <required> path to a template of Mini Mario's head, facing left   }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string mario_vid_loc(parser.get<string>("mario_vid"));
    string mini_marioR_loc(parser.get<string>("mini_marioR"));
    string mini_marioL_loc(parser.get<string>("mini_marioL"));
    //Check of de argumenten niet leeg zijn
    if ( mario_vid_loc.empty() || mini_marioR_loc.empty() || mini_marioL_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }

    //int templ = 1;
    array<Mat, AMOUNTTEMPL> minimario;
    minimario[0] = imread(mini_marioR_loc);
    minimario[1] = imread(mini_marioL_loc);

    //Mat pad = imread("mariopath.png");
    //imshow("Previously recorded path", pad); waitKey(0);

    //https://docs.opencv.org/3.0-beta/modules/videoio/doc/reading_and_writing_video.html
    VideoCapture cap(mario_vid_loc); // open the video
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat frame;
    int frameCount = -1;
    //namedWindow("Frames (press space to pause, any other key to quit)", WINDOW_AUTOSIZE);

    while(cap.read(frame))
    {
        if ( capHeight )
            frameCount++;

        Mat matched = match(frame, minimario, frameCount);
        imshow("Frames (press space to pause, any other key to quit)", matched);
        //Press space to pause, any other key to exit
        int k = waitKey(10);
        if ( k == 32 )
        {
            slider(frame, minimario, frameCount);
        }
        else if ( k == 'c' )
            capHeight = !capHeight;
        else if ( k >= 0 )
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor

    Mat pathCanvas = Mat::zeros(frame.rows, path.size(), CV_8UC1);
    namedWindow("Mario's path visualized", WINDOW_AUTOSIZE);
    Scalar colour = Scalar(255, 255, 255);
    for ( size_t i = 0; i < path.size(); i++ )
    {
        if ( path.at(i).y != 0 )
            polylines(pathCanvas, path, false, colour);
    }
    imshow("Mario's height plotted against time", pathCanvas); waitKey(0);
    imwrite("mariopath.png", pathCanvas);

    return 0;
}
