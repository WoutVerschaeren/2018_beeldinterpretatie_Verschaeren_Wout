#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
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
    //Check of de argumenten niet leeg zijn
    if ( image_gray_loc.empty() || image_colour_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }

    ///Read and show the first image
    Mat img1 = imread(image_gray_loc);
    if ( img1.empty() ){                        //Check if the image can be found
        cerr << "Image 1 not found.";
        return -1;
    }
    imshow("Grayscale image", img1);            //Show the image
    waitKey(0);                                 //Let the user press a key to continue

    ///Read and show the second image
    Mat img2 = imread(image_colour_loc);
    if ( img2.empty() ){                        //Check if the image can be found
        cerr << "Image 2 not found.";
        return -1;
    }
    imshow("Colour image", img2);               //Show the image
    waitKey(0);                                 //Let the user press a key to continue

    vector<Mat> channels;
    split(img2, channels);                      //Split the colour image into three different channels

    ///Show the three different channels, press keys in between
    imshow("Blue channel", channels[0]);
    waitKey(0);
    imshow("Green channel", channels[1]);
    waitKey(0);
    imshow("Red channel", channels[2]);
    waitKey(0);

    ///Convert the colour image to gray scale
    Mat img_gray;
    cvtColor(img2, img_gray, COLOR_BGR2GRAY);
    imshow("This image has been converted to gray scale!",img_gray);
    waitKey(0);

    ///For-loop running through the converted image, row by row and column by column, printing the value of each pixel
    int row;
    int col;
    int value;

    for(row = 0; row < img_gray.rows; row++){
        for(col = 0; col < img_gray.cols; col++){
            //Print the value of the pixel to the command line
            value = (int)img_gray.at<uchar>(row,col);
            cout << value;
            cout << " ";
        }
        cout << endl;
    }
    waitKey(0);

    ///Canvas, rectangle() en circle()
    Mat canvas = Mat::zeros(Size(250,250), CV_8UC3);                        //Create empty canvas (filled with black)
    rectangle(canvas, Point(50,30), Point(100,100), Scalar(255,0,0));       //Rectangle from (50,30) to (100,100) in blue
    rectangle(canvas, Point(100,100), Point(200,200), Scalar(0,255,0));     //Square from (100,100) to (200,200) in red
    circle(canvas, Point(150,150), 50, Scalar(0,0,255), 1);                 //Circle with centre on (150,150) and radius 50, in green, line thickness 1
    line(canvas, Point(220,80), Point(80,220), Scalar(255,255,255), 2);     //Line from (220,80) to (80,220) in white, line thickness 2
    imshow("Rectangle, square, circle, line!",canvas);
    waitKey(0);
}
