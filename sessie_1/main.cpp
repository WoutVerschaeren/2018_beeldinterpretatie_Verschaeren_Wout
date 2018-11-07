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
        "{image_adapted p3|      | <required> path to adapted colour image   }"
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
    string image_adapted_loc(parser.get<string>("image_adapted"));
    //Check of de argumenten niet leeg zijn
    if ( image_colour_loc.empty() || image_bimodal_loc.empty() || image_adapted_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }


    ///1.1: THRESHOLDING: SEGMENTING SKIN PIXELS

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

    Mat mask = Mat::zeros(img1.rows, img1.cols, CV_8UC1);   //Initializing the matrix on all zeroes, 1 channel

    ///Fill the mask using matrix operations
    //The mask is based on a skin colour filter that can be found in the literature
    mask = (RED>95) & (GREEN>40) & (BLUE>20) & ((max(RED,max(GREEN,BLUE)) - min(RED,min(GREEN,BLUE)))>15) & (abs(RED-GREEN)>15) & (RED>GREEN) & (RED>BLUE);

    ///Apply the mask to every channel
    vector<Mat> channels_masked = channels;
    channels_masked[0] = channels[0] & mask;
    channels_masked[1] = channels[1] & mask;
    channels_masked[2] = channels[2] & mask;

    ///Merge the masked channels back into one image
    Mat masked_img(img1.rows, img1.cols, CV_8UC3);  //Create a 3 channel image
    merge(channels_masked,masked_img);

    mask = mask * 255;                          //Set the ones in the mask to maximum brightness
    imshow("Mask", mask);                       //Show the mask
    waitKey(0);

    imshow("Masked image", masked_img);         //Show the image with the mask applied
    waitKey(0);


    ///1.1: THRESHOLDING: SEGMENTING TEXT

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

    ///Apply Contrast Limited Adaptive Histogram Equalization (CLAHE) and threshold again
    Mat result_CLAHE;
    Ptr<CLAHE> clahe_ptr = createCLAHE();
    clahe_ptr->setTilesGridSize(Size(15,15));
    clahe_ptr->setClipLimit(1);
    clahe_ptr->apply(img2_gray.clone(), result_CLAHE);
    threshold(result_CLAHE, img2_thresh, 0, 255, THRESH_OTSU | THRESH_BINARY);
    imshow("Bimodal image thresholded after CLAHE", img2_thresh);
    waitKey(0);


    ///1.2: EROSION AND DILATION

    ///Read and show the third image
    Mat img3 = imread(image_adapted_loc);
    if ( img3.empty() ){                        //Check if the image can be found
        cerr << "Image 3 not found.";
        return -1;
    }
    imshow("Adapted colour image", img3);       //Show the image
    waitKey(0);

    ///Split the image into BGR channels
    vector<Mat> channels3;
    split(img3, channels3);                      //Split the colour image into three different channels
    Mat BLUE3 = channels3[0];                    //channels[0] is BLUE, channels[1] is GREEN and channels[2] is RED
    Mat GREEN3 = channels3[1];
    Mat RED3 = channels3[2];

    Mat mask3 = Mat::zeros(img3.rows, img3.cols, CV_8UC1);   //Initializing the matrix on all zeroes, 1 channel

    ///Fill the mask using matrix operations
    //The mask is based on a skin colour filter that can be found in the literature
    mask3 = (RED3>95) & (GREEN3>40) & (BLUE3>20) & ((max(RED3,max(GREEN3,BLUE3)) - min(RED3,min(GREEN3,BLUE3)))>15) & (abs(RED3-GREEN3)>15) & (RED3>GREEN3) & (RED3>BLUE3);

    ///Opening
    erode(mask3, mask3, Mat(), Point(-1,-1), 2);
    dilate(mask3, mask3, Mat(), Point(-1,-1), 2);
    imshow("Mask after opening", mask3);
    waitKey(0);

    ///Closing
    dilate(mask3, mask3, Mat(), Point(-1,-1), 5);
    erode(mask3, mask3, Mat(), Point(-1,-1), 5);
    imshow("Mask after opening and closing", mask3);
    waitKey(0);

    ///Creating blobs around the mask openings by drawing contours
    vector< vector<Point> > contours;
    findContours(mask3.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    vector< vector<Point> > hulls;
    for(size_t i=0; i<contours.size(); i++){
        vector<Point> hull;
        convexHull(contours[i], hull);
        hulls.push_back(hull);   //Push this hull onto the back of the hulls vector
    }

    drawContours(mask3, hulls, -1, 255, -1);
    imshow("Mask after drawing contours", mask3);
    waitKey(0);

    ///Apply the mask to every channel
    vector<Mat> channels_masked3 = channels3;
    channels_masked3[0] = channels3[0] & mask3;
    channels_masked3[1] = channels3[1] & mask3;
    channels_masked3[2] = channels3[2] & mask3;

    ///Merge the masked channels back into one image and show
    Mat masked_img3(img3.rows, img3.cols, CV_8UC3);     //Create a 3 channel image
    merge(channels_masked3,masked_img3);
    imshow("Masked image (Contoured mask)", masked_img3);                //Show the image with the mask applied
    waitKey(0);
}
