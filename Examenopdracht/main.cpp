#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int AMOUNTTEMPL = 2;
const int BUFSIZE = 5;
const Scalar WHITE = Scalar(255, 255, 255);
const Scalar RED = Scalar(0, 0, 255);

const int slider_max = 100;
int thr;                        //max threshold value
int alpha_slider_thr = 35;      //slider for threshold value

vector<Point> path;
bool capHeight = false;


///Function for the trackbar
static void thr_on_trackbar(int, void *)
{
    thr = alpha_slider_thr;
}

///Checks which template match had the highest score
/*
    maxVals: Array containing the best hits of every checked template

    Returns which match was the most probable
    If no matches were found, returns -1
*/
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
/*
    img: The image that needs to be searched
    templates: An array containing the templates that need to be found
    frameCount: An always increasing number, signifying how many frames have already been "recorded"

    Returns the canvas, which is the current frame with possibly a rectangle around the match
*/
Mat match(Mat img, array<Mat, AMOUNTTEMPL> templates, double frameCount)
{
    Mat result;
    Mat canvas = img.clone();
    array<double, AMOUNTTEMPL> maxVals;
    array<Point, AMOUNTTEMPL> matchLocs;
    int indOfMax;

    ///Match for every template, if the match gets accepted, draw a rectangle around it
    for ( size_t i = 0; i < templates.size(); i++ ) {
        ///Do the matching
        matchTemplate(img.clone(), templates[i], result, TM_SQDIFF_NORMED);
        //SQDIFF represents the best hits with the lowest values: invert!
        result = 1.0-result;

        ///Find the best hit
        double minVal; double maxVal; Point minLoc; Point maxLoc;
        minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

        //Threshold is defined as a percentage
        double thr_norm = (thr*1.0)/100;

        //Does the best hit suffice?
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
            ///Reject the match
            maxVals[i] = 0;
        }
    }

    //Find which match was the most probable
    indOfMax = getIndOfMax(maxVals);
    //Set the text position
    int textX = canvas.rows / 32;
    int textY = canvas.rows * 0.97;

    ///Show whether or not Mario is being "recorded" and save his position if he is
    if ( capHeight )
    {
        if ( indOfMax != -1 )
            path.push_back(Point(frameCount, matchLocs[indOfMax].y));
        else
            path.push_back(Point(frameCount, -1));

        putText(canvas, "Capturing height: on", Point(textX, textY), FONT_HERSHEY_SIMPLEX, 0.6, WHITE, 1);
    }
    else
        putText(canvas, "Capturing height: off", Point(textX, textY), FONT_HERSHEY_SIMPLEX, 0.6, WHITE, 1);


    return canvas;
}

///Function to show the current frame with a trackbar to finetune the template matching threshold
/*
    img: The image that needs to be searched
    templates: An array containing the templates that need to be found
    frameCount: An always increasing number, signifying how many frames have already been "recorded"
*/
void slider(Mat img, array<Mat, 2> templates, double frameCount)
{
    String imTitle = "Edit the threshold for detecting Mario";
    //Create window
    namedWindow(imTitle, WINDOW_AUTOSIZE);
    //Create slider
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

///If Mario was being recorded but there wasn't a match, we injected "holes" in his path (y-pos of -1).
/// This function takes the path with holes and linearly interpolates between the detected points before and after each hole
void interpolatePath()
{
    ///If the first elements of the vector path are invalid (y = -1), delete them
    ///Same for the last elements
    while ( path.front().y < 0 )
        path.erase(path.begin());
    while ( path.back().y < 0 )
        path.erase(path.end());

    ///Loop through the path, change the y coordinates of the invalid elements to interpolated values
    //Start on i=1, since first element will never be invalid
    for ( size_t i = 1; i < path.size(); i++ )
    {
        if ( path.at(i).y < 0 )
        {
            //Find the next valid element
            int j = 1;
            while ( path.at(i+j).y < 0 )
                j++;

            //Previous valid element and the y-difference between it and the next valid element
            int previous = path.at(i-1).y;
            int difference = path.at(i+j).y - previous;

            //Loop through all the invalid elements, linearly interpolate their y-values
            //between the previous and next valid elements
            int k = 0;
            while ( k < j )
            {
                path.at(i+k).y = previous + round( (k+1)*difference/(j+1) );
                k++;
            }

            //Skip all the elements we just interpolated plus the next valid element
            i += k;
        }
    }
}

///Function to count Mario's jumps based on a given path
/*
    jumpPoints: A vector in which the location of the jumps will be saved

    Return how many jumps were detected
*/
int countJumps(vector<Point>& jumpPoints)
{
    array<int, BUFSIZE> buffer;
    array<bool, 3> jump = {false, false, false};
    array<bool, 3> fall = {false, false, false};
    bool jumping = false;
    int jumpCount = 0;

    ///Fill the buffer initially with the first BUFSIZE elements of the path
    for ( size_t i = 0; i < BUFSIZE; i++ )
        buffer[i] = path.at(i).y;

    ///Check if there is a jump compared to BUFSIZE elements ago, replace element in buffer with current
    for ( size_t i = BUFSIZE; i < path.size(); i++ )
    {
        //If a previous jump hasn't been registered
        if ( !jumping )
        {
            //Shift the "certainty" array (See below)
            jump[0] = jump[1];
            jump[1] = jump[2];

            //If the difference between this y and the one BUFSIZE elements ago is greater than 35
            if ( path.at(i).y - buffer[(i)%BUFSIZE] < -35 )
                jump[2] = true;
            else
                jump[2] = false;

            //To filter out noise: necessary to register 3 jumps in succession
            if ( jump[0] == 1 && jump[1] == 1 && jump[2] == 1 )
            {
                jumpCount++;
                jumpPoints.push_back(path.at(i));
                jumping = true;                     //Mario is jumping, don't register another jump until he has come back down, see below
                jump = {false, false, false};
            }
        }
        //Analogous to the above
        else
        {
            fall[0] = fall[1];
            fall[1] = fall[2];

            if ( path.at(i).y - buffer[(i)%BUFSIZE] > 18 )
                fall[2] = true;
            else
                fall[2] = false;

            if ( fall[0] == 1 && fall[1] == 1 && fall[2] == 1 )
            {
                jumping = false;
                fall = {false, false, false};
            }
        }

        buffer[i%BUFSIZE] = path.at(i).y;
    }

    return jumpCount;
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

    ///Bundle all templates in an array
    array<Mat, AMOUNTTEMPL> minimario;
    minimario[0] = imread(mini_marioR_loc);
    minimario[1] = imread(mini_marioL_loc);

    //https://docs.opencv.org/3.0-beta/modules/videoio/doc/reading_and_writing_video.html
    VideoCapture cap(mario_vid_loc); // Open the video
    if(!cap.isOpened())  // Check if we succeeded
        return -1;

    Mat frame;
    int frameCount = -1;

    while(cap.read(frame))
    {
        if ( capHeight )
            frameCount++;

        Mat matched = match(frame, minimario, frameCount);
        imshow("Super Mario World", matched);
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
    destroyWindow("Super Mario World");
    // the camera will be deinitialized automatically in VideoCapture destructor

    namedWindow("Mario's height plotted against time", WINDOW_AUTOSIZE);
    /*
    Uncomment this to view why interpolation is necessary

    Mat pathCanvas = Mat::zeros(frame.rows, path.size(), CV_8UC1);
    polylines(pathCanvas, path, false, WHITE);
    imshow("Mario's height plotted against time BAD", pathCanvas); waitKey(0);
    */

    ///Interpolate the path, count the jumps and save their locations
    interpolatePath();
    vector<Point> jumpPoints;
    int jumpCount = countJumps(jumpPoints);

    ///Draw an image showing the analysis and save it
    Mat pathCanvas = Mat::zeros(frame.rows, path.size(), CV_8UC3);
    polylines(pathCanvas, path, false, WHITE);
    for ( size_t i = 0; i < jumpPoints.size(); i++ )
        circle(pathCanvas, jumpPoints[i], 5, RED);
    putText(pathCanvas, "Jumps: " + to_string(jumpCount), Point(20, 50), FONT_HERSHEY_SIMPLEX, 1, RED, 1);

    imshow("Mario's height plotted against time", pathCanvas); waitKey(0);
    imwrite("mariopath.png", pathCanvas);


    return 0;
}
