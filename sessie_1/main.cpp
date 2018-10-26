#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?  |      | print this message   }"
        "{image_colour p1 |      | <required> path to colour image   }"
        "{image_bimodal p2|      | <required> path to bimodal image  }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string image_colour_loc(parser.get<string>("image_colour"));
    string image_bimodal_loc(parser.get<string>("image_bimodal"));
    //Check of de argumenten niet leeg zijn
    if ( image_colour_loc.empty() || image_bimodal_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }

    ///SEGMENTING SKIN PIXELS

    ///Read and show the first image
    Mat img1 = imread(image_colour_loc);
    if ( img1.empty() ){                        //Check if the image can be found
        cerr << "Image 1 not found.";
        return -1;
    }
    imshow("Colour image", img1);               //Show the image
    waitKey(0);

    ///Split the image into BGR channels
    vector<Mat> channels;
    split(img1, channels);                      //Split the colour image into three different channels
    Mat BLUE = channels[0];                     //channels[0] is BLUE, channels[1] is GREEN and channels[2] is RED
    Mat GREEN = channels[1];
    Mat RED = channels[2];

    Mat mask = Mat::zeros(img1.rows, img1.cols, CV_8UC1);   //Initializing the matrix on all zeroes

    ///Fill the mask using matrix operations
    //The mask is based on a skin colour filter that can be found in the literature
    mask = (RED>95) & (GREEN>40) & (BLUE>20) & ((max(RED,max(GREEN,BLUE)) - min(RED,min(GREEN,BLUE)))>15) & (abs(RED-GREEN)>15) & (RED>GREEN) & (RED>BLUE);

    ///Apply the mask to every channel
    vector<Mat> channels_masked = channels;
    channels_masked[0] = channels[0] & mask;
    channels_masked[1] = channels[1] & mask;
    channels_masked[2] = channels[2] & mask;

    ///Merge the masked channels back into one image
    Mat masked_img(img1.rows, img1.cols, CV_8UC3);
    merge(channels,masked_img);

    mask = mask * 255;                          //Set the ones in the mask to maximum brightness
    imshow("Mask", mask);                       //Show the mask
    waitKey(0);

    imshow("Masked image", masked_img);         //Show the image with the mask applied
    waitKey(0);


    ///SEGMENTING TEXT

    ///Read and show the second image
    Mat img2 = imread(image_bimodal_loc);
    if ( img2.empty() ){                        //Check if the image can be found
        cerr << "Image 2 not found.";
        return -1;
    }
    imshow("Bimodal image", img2);               //Show the image
    waitKey(0);

    ///Convert the colour image to gray scale
    Mat img2_gray;
    cvtColor(img2, img2_gray, COLOR_BGR2GRAY);
    imshow("Bimodal image converted to gray scale",img2_gray);
    waitKey(0);

    ///Threshold the image with plain OTSU thresholding
    Mat img2_thresh;
    threshold(img2_gray, img2_thresh, 0, 255, THRESH_OTSU | THRESH_BINARY);
    imshow("Bimodal image thresholded", img2_thresh);
    waitKey(0);

    ///Equalize the colour histogram and threshold again
    Mat img2_gray_eq;
    equalizeHist(img2_gray.clone(),img2_gray_eq);
    threshold(img2_gray_eq, img2_thresh, 0, 255, THRESH_OTSU | THRESH_BINARY);
    imshow("Bimodal image thresholded after histogram equalization", img2_thresh);
    waitKey(0);
/*
    Mat result_CLAHE;
    Ptr<CLAHE> clahe_ptr = createCLAHE();
    clahe_ptr->setTileGridSize(Size(15,15));
    clahe_ptr->setClipLimit(1);
    clahe_ptr->apply(gray_ticket.clone(), result_CLAHE);
    threshold();
    */


    /*
    Masken
    erode en dilate moeten op mask gebeuren
    erode(mask, mask, Mat(), Point(-1,-1), 2);
    dilate();
    imshow

    dilate();
    erode();
    imshow

    vector< vector<Point> > contours;
    findContours(mask.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    vector< vector<Point> > hulls;
    for(size_t i=0, i<contours.size(), i++){
        vector<Point> hull;
        convexHull(contours[i], hull);
        hulls.push_back(hull)   //achteraan matrix bijduwen
    }

    drawContours(mask, hulls, -1, 255, -1);
    imshow mask
}
