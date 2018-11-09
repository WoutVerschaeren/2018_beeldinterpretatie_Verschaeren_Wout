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
        vector<Mat> channels;
        //Split the colour image into three different channels
        split(img, channels);
        //channels[0] is BLUE, channels[1] is GREEN and channels[2] is RED
        Mat BLUE = channels[0];
        Mat GREEN = channels[1];
        Mat RED = channels[2];

        //Initializing the matrix on all zeroes, 1 channel
        Mat mask = Mat::zeros(img.rows, img.cols, CV_8UC1);

        ///Fill the mask using matrix operations
        mask = (RED>100) & (BLUE<50) & (GREEN<50);

        ///Apply the mask to every channel
        vector<Mat> channels_masked = channels;
        channels_masked[0] = channels[0] & mask;
        channels_masked[1] = channels[1] & mask;
        channels_masked[2] = channels[2] & mask;

        ///Merge the masked channels back into one image
        //Create a 3 channel image
        Mat masked_img(img.rows, img.cols, CV_8UC3);
        merge(channels_masked,masked_img);

        //Set the ones in the mask to maximum brightness
        mask = mask * 255;
        //Show the mask
        imshow("Mask " + to_string(i), mask);
        waitKey(0);

        //Show the image with the mask applied
        imshow("Masked image " + to_string(i), masked_img);
        waitKey(0);


        ///2.2: SEGMENTING IN HSV COLOUR SPACE

        ///Split the image into HSV channels
        Mat img_hsv;
        cvtColor(img, img_hsv, COLOR_BGR2HSV);
        //Split the HSV image into three different channels
        split(img_hsv, channels);
        //channels[0] is Hue, channels[1] is Saturation and channels[2] is Value (Intensity)
        Mat H = channels[0];
        Mat S = channels[1];
        Mat V = channels[2];

        //Initializing the matrices on all zeroes, 1 channel
        Mat mask_hsv = Mat::zeros(img_hsv.rows, img_hsv.cols, CV_8UC1);
        Mat mask_hsv1 = Mat::zeros(img_hsv.rows, img_hsv.cols, CV_8UC1);
        Mat mask_hsv2 = Mat::zeros(img_hsv.rows, img_hsv.cols, CV_8UC1);

        ///Fill the masks using matrix operations
        mask_hsv1 = (H<10);
        mask_hsv2 = (H>168);
        mask_hsv = mask_hsv1 | mask_hsv2;

        ///Apply the mask to every channel
        vector<Mat> channels_masked_hsv = channels;
        channels_masked_hsv[0] = channels[0] & mask_hsv;
        channels_masked_hsv[1] = channels[1] & mask_hsv;
        channels_masked_hsv[2] = channels[2] & mask_hsv;

        ///Merge the masked channels back into one image
        //Create a 3 channel image
        Mat masked_img_hsv(img_hsv.rows, img_hsv.cols, CV_8UC3);
        merge(channels_masked_hsv,masked_img_hsv);

        //Set the ones in the mask to maximum brightness
        mask_hsv = mask_hsv * 255;
        //Show the mask
        imshow("Mask HSV " + to_string(i), mask_hsv);
        waitKey(0);

        //Show the image with the mask applied
        imshow("Masked image HSV " + to_string(i), masked_img_hsv);
        waitKey(0);

        namedWindow("Linear Blend", WINDOW_AUTOSIZE); // Create Window
        char TrackbarName[50];
        sprintf( TrackbarName, "Alpha x %d", alpha_slider_max1 );
        createTrackbar( TrackbarName, "Linear Blend", &alpha_slider1, alpha_slider_max1, on_trackbar1 );
        on_trackbar1( alpha_slider1, 0 );
        waitKey(0);


        i++;
    }
}
