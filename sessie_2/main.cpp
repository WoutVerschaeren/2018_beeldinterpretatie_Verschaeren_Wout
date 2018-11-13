#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int alpha_slider_max1 = 180;
int alpha_slider1 = 168;
const int alpha_slider_max2 = 180;
int alpha_slider2 = 10;
const int alpha_slider_max3 = 254;
int alpha_slider3 = 115;

double alpha;
double beta;
Mat src1;
Mat src2;
Mat dst;


static void on_trackbar1( int, void* )
{
   alpha = (double) alpha_slider1/alpha_slider_max1;
   beta = ( 1.0 - alpha );
   addWeighted( src1, alpha, src2, beta, 0.0, dst);
   imshow( "Linear Blend", dst );
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

    ///Fill the masks using matrix operations
    maskHSV1 = (H<10);
    maskHSV2 = (H>168);
    maskHSV = maskHSV1 | maskHSV2;

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

/*
int hsvsegment_slider(Mat sign_image)
{
    namedWindow("Window: ESC to exit"); // Create Window
    resizeWindow("Window: ESC to exit", 200, 200);//resize it (it gets auto-sized later)
    createTrackbar("max H", "Window: ESC to exit", &alpha_slider1, alpha_slider_max1, on_trackbar1); //add on the three trackbars
    createTrackbar("min H", "Window: ESC to exit", &alpha_slider2, alpha_slider_max2, on_trackbar2);
    createTrackbar("S", "Window: ESC to exit", &alpha_slider3, alpha_slider_max3, on_trackbar3);
    on_trackbar1(alpha_slider1,0);
    on_trackbar2(alpha_slider2,0);
    on_trackbar3(alpha_slider3,0);

    Mat segmented_image;
    Mat concat_image;
    while (true)
    {
        segmented_image = hsvsegment_mat(sign_image);   //segment the image using the hsvsegment_mat function (which also used the values h1,h2,s1)
        hconcat(sign_image, segmented_image, concat_image); //concatenate original and segmented image
        imshow("Window: ESC to exit",concat_image); //show result
        int key = waitKey(10);

        if (key == 27) //exit if ESC is pressed
        {
            return 0;
        }
    }
}
*/


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

        ///Split the image into HSV channels
        Mat masked_imgHSV = HSVSegment(img);
        /*
        Mat img_hsv;
        cvtColor(img, img_hsv, COLOR_BGR2HSV);
        //Split the HSV image into three different channels
        vector<Mat> channelsHSV;
        split(img_hsv, channelsHSV);
        //channels[0] is Hue, channels[1] is Saturation and channels[2] is Value (Intensity)
        Mat H = channelsHSV[0];
        Mat S = channelsHSV[1];
        Mat V = channelsHSV[2];

        //Initializing the matrices on all zeroes, 1 channel
        Mat mask_hsv = Mat::zeros(img_hsv.rows, img_hsv.cols, CV_8UC1);
        Mat mask_hsv1 = Mat::zeros(img_hsv.rows, img_hsv.cols, CV_8UC1);
        Mat mask_hsv2 = Mat::zeros(img_hsv.rows, img_hsv.cols, CV_8UC1);

        ///Fill the masks using matrix operations
        mask_hsv1 = (H<10);
        mask_hsv2 = (H>168);
        mask_hsv = mask_hsv1 | mask_hsv2;

        ///Apply the mask to every channel
        vector<Mat> channels_masked_hsv = channelsBGR;
        channels_masked_hsv[0] = channelsBGR[0] & mask_hsv;
        channels_masked_hsv[1] = channelsBGR[1] & mask_hsv;
        channels_masked_hsv[2] = channelsBGR[2] & mask_hsv;

        ///Merge the masked channels back into one image
        //Create a 3 channel image
        Mat masked_img_hsv(img_hsv.rows, img_hsv.cols, CV_8UC3);
        merge(channels_masked_hsv,masked_img_hsv);
        */

        //Show the image with the mask applied
        imshow("Masked image HSV " + to_string(i), masked_imgHSV);
        waitKey(0);

        /*
        namedWindow("Linear Blend", WINDOW_AUTOSIZE); // Create Window
        char TrackbarName[50];
        sprintf( TrackbarName, "Alpha x %d", alpha_slider_max1 );
        createTrackbar( TrackbarName, "Linear Blend", &alpha_slider1, alpha_slider_max1, on_trackbar1 );
        on_trackbar1( alpha_slider1, 0 );
        waitKey(0);
        */


        i++;
    }
}
