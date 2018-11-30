#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

vector<Point2f> strawberryPts;
vector<Point2f> backgroundPts;

void strawberryCb(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        strawberryPts.push_back(Point2f(x, y));
        cout << "Strawberry point added at position (" << x << ", " << y << ")" << endl;
    }
    else if  ( event == EVENT_RBUTTONDOWN )
    {
        if ( !strawberryPts.empty() )
        {
            cout << "Last strawberry point removed: position (" << strawberryPts.back().x << ", " << strawberryPts.back().y << ")" << endl;
            strawberryPts.pop_back();
        }
    }
    else if ( event == EVENT_MBUTTONDOWN )
    {
        cout << "StrawberryPts: " << strawberryPts << endl;
    }
}

void backgroundCb(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        backgroundPts.push_back(Point2f(x, y));
        cout << "Background point added at position (" << x << ", " << y << ")" << endl;
    }
    else if  ( event == EVENT_RBUTTONDOWN )
    {
        if ( !backgroundPts.empty() )
        {
            cout << "Last background point removed: position (" << backgroundPts.back().x << ", " << backgroundPts.back().y << ")" << endl;
            backgroundPts.pop_back();
        }
    }
    else if ( event == EVENT_MBUTTONDOWN )
    {
        cout << "BackgroundPts: " << backgroundPts << endl;
    }
}


int clickableImg(string img_loc, string title, void (*callbackFunc)(int, int, int, int, void*))
{
    ///Read and show the image
    Mat img = imread(img_loc);
    //Check if the images can be found
    if ( img.empty() ){
        cerr << "One or more images can't be found.";
        return -1;
    }

    //Blur the image a bit so we don't accidentally click the black points of the strawberry
    GaussianBlur(img, img, Size(5, 5), 0, 0 );

    //Create a window
    namedWindow(title, WINDOW_AUTOSIZE);
    //Set the callback function
    setMouseCallback(title, callbackFunc, &img);
    //Show the image
    imshow(title, img);    waitKey(0);

    return 0;
}

int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?  |      | print this message   }"
        "{image1 p1 |     | <required> path to image of strawberries  }"
        "{image2 p2|      | <required> path to image of strawberries  }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string image1_loc(parser.get<string>("image1"));
    string image2_loc(parser.get<string>("image2"));
    //Check of de argumenten niet leeg zijn
    if ( image1_loc.empty() || image2_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }


    ///5.1: MOUSE CALLBACKS: CLICKING POINTS

    ///Read and show the images, set a callback function on them that registers left and right clicks
    clickableImg(image1_loc, "Click the strawberries! (LMB to add, RMB to delete most recent, MMB to show all)", strawberryCb);
    cout << "StrawberryPts: " << strawberryPts << endl;
    clickableImg(image1_loc, "Click the background! (LMB to add, RMB to delete most recent, MMB to show all)", backgroundCb);
    cout << "BackgroundPts: " << backgroundPts << endl;


    /*
    Mat img_hsv;
    cvtColor(img, img_hsv, COLOR_BGR2HSV);

    Mat trainingDataForeground(strawberryPts.size(), 3, CV_32FC1);
    Mat labels_fg = Mat::ones(strawberryPts.size(), 1, CV_32FC1);

    for( int i = 0; i < strawberryPts.size(); i++ )
    {
        Vec3b descriptor = img_hsv.at<Vec3b>(strawberryPts[i].y, strawberryPts[i].x);
        trainingDataForeground.at<float>(i, 0) = descriptor[0];
        trainingDataForeground.at<float>(i, 1) = descriptor[1];
        trainingDataForeground.at<float>(i, 2) = descriptor[2];
    }

    //Zelfde voor bg: Mat trainingDataBackground(backgroundPts.size(), 3, CV_32FC1);

    Mat trainingData, labels;
    vconcat(trainingDataForeground, trainingDataBackground, trainingData);
    vconcat(labels_fg, labels_bg, labels);
    */

    /*
    ///Training a 1 Nearest Neighbour Classifier
    Ptr<KNearest> kNN = KNearest::create();
    Ptr<TrainData> trainingDataKNN = TrainData::create(trainingData, ,);
    */

    //Verbeteren? Groene kanaal wegflikkeren!
}
