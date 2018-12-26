#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

///Mask the image so all white pixels are deleted
Mat maskNoWhite(Mat img)
{
    //Split the image into BGR channels
    vector<Mat> channels;
    split(img, channels);                           //Split the colour image into three different channels

    Mat mask = (channels[0] < 255) & (channels[1] < 255) & (channels[2] < 255);

    //Apply the mask to every channel
    vector<Mat> channels_masked = channels;
    channels_masked[0] = channels[0] & mask;
    channels_masked[1] = channels[1] & mask;
    channels_masked[2] = channels[2] & mask;

    //Merge the masked channels back into one image
    Mat masked_img(img.rows, img.cols, CV_8UC3);    //Create a 3 channel image
    merge(channels_masked,masked_img);

    return masked_img;
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

    if ( maxVal >= 0.2 )
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
    minimario = maskNoWhite(minimario);

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
            waitKey(0);
        }
        else if ( k >= 0 )
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
