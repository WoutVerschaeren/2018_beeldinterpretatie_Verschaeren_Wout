#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

///Finds a match in the source image src using the template tmpl
void matchSingle(Mat src, Mat tmpl)
{
    Mat result;

    ///Do the matching and normalize
    matchTemplate(src.clone(), tmpl, result, TM_SQDIFF);
    normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
    //SQDIFF represents the best hits with the lowest values: invert!
    result = 1-result;
    imshow("Match map single", result); waitKey(0);

    ///Find the best hit
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
    matchLoc = maxLoc;

    Mat mask;
    threshold(result, mask, 0.9, 1, THRESH_BINARY);
    mask.convertTo(mask, CV_8UC1);
    mask *= 255;
    imshow("Thresholded match map single", mask); waitKey(0);

    Mat best_hit = src.clone();
    rectangle(best_hit, matchLoc, Point(matchLoc.x + tmpl.cols , matchLoc.y + tmpl.rows), Scalar::all(0), 2, 8, 0);
    imshow("Result", result);
    imshow("Best hit", best_hit);
    waitKey(0);
}

void matchAll(Mat src, Mat tmpl)
{
    Mat result;

    ///Do the matching and normalize
    matchTemplate(src.clone(), tmpl, result, TM_CCORR);
    normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
    imshow("Match map all", result); waitKey(0);

    ///Find the min and max
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

    Mat mask;
    //0.602 is an empirical value: low enough to detect all matches, high enough to not let mask openings touch each other
    inRange(result, maxVal * 0.602, maxVal, mask);
    mask.convertTo(mask, CV_8UC1);
    mask *= 255;
    imshow("Thresholded match map all", mask); waitKey(0);

    //Multiple hits
    Mat all_hits = src.clone();
    vector< vector<Point> > contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    for(int i=0; i < contours.size(); i++)
    {
        Rect region = boundingRect(contours[i]);
        Mat temp = result(region);
        Point maxLoc;
        minMaxLoc(temp, NULL, NULL, NULL, &maxLoc);
        rectangle(all_hits, Point(region.x + maxLoc.x , region.y + maxLoc.y), Point(region.x + maxLoc.x + tmpl.cols , region.y + maxLoc.y + tmpl.rows), Scalar::all(0), 2, 8, 0);
    }
    imshow("All hits", all_hits);
    waitKey(0);
}

int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?   |      | print this message   }"
        "{image_source p1  |      | <required> path to image in which the template needs to be found   }"
        "{image_template p2|      | <required> path to image of the template  }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string image_source_loc(parser.get<string>("image_source"));
    string image_template_loc(parser.get<string>("image_template"));
    //Check of de argumenten niet leeg zijn
    if ( image_source_loc.empty() || image_template_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }

    ///3.1: TEMPLATE MATCHING: ONE MATCH

    ///Read and show the source image
    Mat img_src = imread(image_source_loc);
    //Check if the image can be found
    if ( img_src.empty() ){
        cerr << "Source image not found.";
        return -1;
    }
    //Show the image
    imshow("Source image", img_src);
    waitKey(0);

    ///Convert the source image to gray scale
    Mat img_src_gray;
    cvtColor(img_src, img_src_gray, COLOR_BGR2GRAY);


    ///Read and show the template image
    Mat img_tmpl = imread(image_template_loc);
    //Check if the image can be found
    if ( img_tmpl.empty() ){
        cerr << "Source image not found.";
        return -1;
    }
    //Show the image
    imshow("Template image", img_tmpl);
    waitKey(0);

    ///Convert the template image to gray scale
    Mat img_tmpl_gray;
    cvtColor(img_tmpl, img_tmpl_gray, COLOR_BGR2GRAY);


    ///Find a match in the source image
    matchSingle(img_src, img_tmpl);
    ///Find all matches
    matchAll(img_src, img_tmpl);
}
