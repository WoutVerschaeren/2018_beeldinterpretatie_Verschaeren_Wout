#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int slider_max_H = 180;
const int slider_max_S = 255;

int alpha_slider_uH = 10;       //upper value for Hue
int alpha_slider_lH = 168;      //lower value for Hue
int alpha_slider_maxS = 115;    //Saturation

int uH = 10;                //upper value for Hue
int lH = 168;               //lower value for Hue
int maxS = 115;             //max Saturation


static void uH_on_trackbar(int, void *)
{
    uH = alpha_slider_uH;
}
static void lH_on_trackbar(int, void *)
{
    lH = alpha_slider_lH;
}

static void S_on_trackbar(int, void *)
{
    maxS = alpha_slider_maxS;
}


Mat HSVSegment(Mat img)
{
    //Split the image into three different channels
    vector<Mat> channelsBGR;
    split(img, channelsBGR);
    //channels[0] is BLUE, channels[1] is GREEN and channels[2] is RED
    Mat BLUE = channelsBGR[0];
    Mat GREEN = channelsBGR[1];
    Mat RED = channelsBGR[2];

    Mat imgHSV;
    cvtColor(img, imgHSV, COLOR_BGR2HSV);
    //Split the image into three different channels
    vector<Mat> channelsHSV;
    split(imgHSV, channelsHSV);
    //channelsHSV[0] is Hue, channelsHSV[1] is Saturation and channelsHSV[2] is Value (Intensity)
    Mat H = channelsHSV[0];
    Mat S = channelsHSV[1];
    Mat V = channelsHSV[2];

    //Initializing the matrices on all zeroes, 1 channel
    Mat maskHSV = Mat::zeros(imgHSV.rows, imgHSV.cols, CV_8UC1);
    Mat maskHSV1 = Mat::zeros(imgHSV.rows, imgHSV.cols, CV_8UC1);
    Mat maskHSV2 = Mat::zeros(imgHSV.rows, imgHSV.cols, CV_8UC1);
    Mat maskHSV3 = Mat::zeros(imgHSV.rows, imgHSV.cols, CV_8UC1);

    ///Fill the masks using matrix operations
    maskHSV1 = (H < uH);
    maskHSV2 = (H > lH);
    maskHSV3 = (S < maxS);
    maskHSV = maskHSV1 | maskHSV2 | maskHSV3;

    ///Opening to reduce noise on the mask
    erode(maskHSV, maskHSV, Mat(), Point(-1,-1), 2);
    dilate(maskHSV, maskHSV, Mat(), Point(-1,-1), 2);

    ///Apply the mask to every channel
    vector<Mat> channels_maskedHSV = channelsHSV;
    channels_maskedHSV[0] = channelsBGR[0] & maskHSV;
    channels_maskedHSV[1] = channelsBGR[1] & maskHSV;
    channels_maskedHSV[2] = channelsBGR[2] & maskHSV;

    ///Merge the masked channels back into one image and show
    //Create a 3 channel image
    Mat masked_imgHSV(imgHSV.rows, imgHSV.cols, CV_8UC3);
    merge(channels_maskedHSV,masked_imgHSV);

    //Return the image with the mask applied
    return masked_imgHSV;
}

int slider(Mat img, int i)
{
    String imTitle = "Masked image HSV " + to_string(i) + " (press enter to continue)";
    namedWindow(imTitle, WINDOW_AUTOSIZE); // Create Window
    //Create H sliders
    createTrackbar("Upper H", imTitle, &alpha_slider_uH, slider_max_H, uH_on_trackbar);
    createTrackbar("Lower H", imTitle, &alpha_slider_lH, slider_max_H, lH_on_trackbar);
    //Create S slider
    createTrackbar("Max S", imTitle, &alpha_slider_maxS, slider_max_S, S_on_trackbar);

    uH_on_trackbar(alpha_slider_uH, 0);
    lH_on_trackbar(alpha_slider_lH, 0);
    S_on_trackbar(alpha_slider_maxS, 0);

    Mat imgHSV;
    Mat imgConcat;
    while (true)
    {
        //Split the image into HSV channels
        Mat masked_imgHSV = HSVSegment(img);

        //Show the image with the mask applied
        imshow(imTitle, masked_imgHSV);

        int key = waitKey(10);
        if ( key == 13 ) //enter
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
        "{sign1 p1 |      | <required> path to an image of a traffic sign   }"
        "{sign2 p2 |      | <required> path to an image of a traffic sign  }"
        "{sign3 p3 |      | <required> path to an image of a traffic sign   }"
        "{sign4 p4 |      | <required> path to an image of a traffic sign  }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string sign1_loc(parser.get<string>("sign1"));
    string sign2_loc(parser.get<string>("sign2"));
    string sign3_loc(parser.get<string>("sign3"));
    string sign4_loc(parser.get<string>("sign4"));
    //Check that the arguments aren't empty
    if ( sign1_loc.empty() || sign2_loc.empty() || sign3_loc.empty() || sign4_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }

    //Create a std::list of the image locations
    std::list<string> locs =    {   sign1_loc,
                                    sign2_loc,
                                    sign3_loc,
                                    sign4_loc
                                };

    //Create an iterator of std::list
    std::list<string>::iterator it;

    int i = 1;

    // Make iterate point to begining and incerement it one by one till it reaches the end of list.
    for (it = locs.begin(); it != locs.end(); it++)
    {
        ///Read and show the image
        Mat img = imread(*it);
        //Check if the image can be found
        if ( img.empty() ){
            cerr << "Image 1 not found.";
            return -1;
        }
        //Show the image
        imshow("Traffic sign " + to_string(i), img);
        waitKey(0);


        ///2.1: SEGMENTING IN BGR COLOUR SPACE

        ///Split the image into BGR channels
        //Split the colour image into three different channels
        vector<Mat> channelsBGR;
        split(img, channelsBGR);
        //channels[0] is BLUE, channels[1] is GREEN and channels[2] is RED
        Mat BLUE = channelsBGR[0];
        Mat GREEN = channelsBGR[1];
        Mat RED = channelsBGR[2];

        //Initializing the matrix on all zeroes, 1 channel
        Mat mask = Mat::zeros(img.rows, img.cols, CV_8UC1);

        ///Fill the mask using matrix operations
        mask = (RED>100) & (BLUE<50) & (GREEN<50);

        ///Apply the mask to every channel
        vector<Mat> channels_masked = channelsBGR;
        channels_masked[0] = channelsBGR[0] & mask;
        channels_masked[1] = channelsBGR[1] & mask;
        channels_masked[2] = channelsBGR[2] & mask;

        ///Merge the masked channels back into one image
        //Create a 3 channel image
        Mat masked_img(img.rows, img.cols, CV_8UC3);
        merge(channels_masked,masked_img);

        /*
        //Set the ones in the mask to maximum brightness
        mask = mask * 255;
        //Show the mask
        imshow("Mask " + to_string(i), mask);
        waitKey(0);
        */

        //Show the image with the mask applied
        imshow("Masked image " + to_string(i), masked_img);
        waitKey(0);


        ///2.2: SEGMENTING IN HSV COLOUR SPACE

        //Split the image into HSV channels
        Mat masked_imgHSV = HSVSegment(img);

        //Show the image with the mask applied
        imshow("Masked image HSV " + to_string(i), masked_imgHSV);
        waitKey(0);


        ///2.4: TRACKBARS
        slider(img, i);


        i++;
    }
}
