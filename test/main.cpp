#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    cout << "Hello world!" << endl;

    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?  |      | print this message   }"
        "{image_gray p1   |      | <required> path to gray image   }"
        "{image_colour p2 |      | <required> path to colour image   }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    ///Collect data from arguments
    string image_gray_loc(parser.get<string>("image_gray"));
    string image_colour_loc(parser.get<string>("image_colour"));
    if ( image_gray_loc.empty() || image_colour_loc.empty() ){
        cerr << "There's something wrong with your arguments.";
        parser.printMessage();
        return -1;
    }

    ///Read the first image
    Mat img1 = imread(image_gray_loc);
    if ( img1.empty() ){
        cerr << "Image 1 not found.";
        return -1;
    }
    imshow("Grayscale image", img1);
    waitKey(0);

    ///Read the second image
    Mat img2 = imread(image_colour_loc);
    if ( img2.empty() ){
        cerr << "Image 2 not found.";
        return -1;
    }
    imshow("Colour image", img2);
    waitKey(0);

    vector<Mat> channels;
    split(img2, channels);
    imshow("Blue channel", channels[0]);
    ///Zelfde voor G en R

    Mat img_gray;
    cvtColor(img2, img_gray, COLOR_BGR2GRAY);
    imshow(img_gray);
    waitKey(0);

    ///For-loop doorheen afbeelding per pixel (per row en per kolom)

    ///Canvas, rectangle() en circle()
}
