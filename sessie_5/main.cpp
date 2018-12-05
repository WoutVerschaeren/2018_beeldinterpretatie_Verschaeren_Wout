#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>

using namespace std;
using namespace cv;
using namespace cv::ml;

vector<Point2f> strawberryPts;
vector<Point2f> backgroundPts;

///Mouse callback function for the foreground
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

///Mouse callback function for the background
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

Mat readImg(string img_loc)
{
    ///Read the image
    Mat img = imread(img_loc);
    //Check if the images can be found
    if ( img.empty() ){
        cerr << "One or more images can't be found.";
        Mat err;
        return err;
    }

    return img;
}


///Show an image, make it clickable (see above callback functions)
int clickableImg(string img_loc, string title, void (*callbackFunc)(int, int, int, int, void*))
{
    ///Read and show the image
    Mat img = readImg(img_loc);

    //Create a window
    namedWindow(title, WINDOW_AUTOSIZE);
    //Set the callback function
    setMouseCallback(title, callbackFunc, &img);
    //Show the image
    imshow(title, img);    waitKey(0);
    destroyWindow(title);

    return 0;
}

///Train a K-Nearest-Neighbour classifier
/*
    trainingData: vector containing all the training points
    labels: vector containing the descriptors of the training points
*/
Ptr<KNearest> trainkNN(Mat trainingData, Mat labels)
{
    Ptr<KNearest> kNN = KNearest::create();
    Ptr<TrainData> trainingDataKNN = TrainData::create(trainingData, SampleTypes::ROW_SAMPLE, labels);

    kNN->setIsClassifier(true);
    kNN->setAlgorithmType(KNearest::Types::BRUTE_FORCE);
    kNN->setDefaultK(3);

    kNN->train(trainingDataKNN);

    cout << "Hier geraakt " << endl;
    return kNN;
}

Mat classify(Mat img, Ptr<KNearest> classifier)
{
    Mat mask = Mat::zeros(img.rows, img.cols, CV_8UC1);
    Mat result;

    cout << "Hier geraakt " << endl;
    for ( int i = 0; i < img.rows; i++ )
    {
        Mat allPixels(img.cols, 3, CV_32FC1);

        for ( int j = 0; j < img.cols; j++ )
        {
            Vec3b currPixel = img.at<Vec3b>(i, j);
            /// NAMEN VERANDEREN

            allPixels.at<float>(j, 0) = currPixel[0];
            allPixels.at<float>(j, 1) = currPixel[1];
            allPixels.at<float>(j, 2) = currPixel[2];

            classifier->findNearest(allPixels, classifier->getDefaultK(), result);

            if ( result.at<float>(j) == 1)
            {
                mask.at<uchar>(i, j) = 255;
            }
        }
    }

    //imshow("Tetten", result);    waitKey(0);
    cout << "Hier geraakt " << endl;
    imshow("Tetters", mask);    waitKey(0);
    return result;
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


    ///5.2: DETERMINING A DESCRIPTOR: HSV IMAGE

    ///Convert the image to HSV and blur
    Mat img = readImg(image1_loc);
    Mat img_hsv;
    cvtColor(img, img_hsv, COLOR_BGR2HSV);

    //Blur the image a bit so the clicked points arent using the black dots on the strawberry
    GaussianBlur(img_hsv, img_hsv, Size(5, 5), 0, 0 );

    ///Collect the training data and determine a label
    //Foreground
    Mat trainingDataForeground(strawberryPts.size(), 3, CV_32FC1);
    //Label of aforeground pixel is a 1
    Mat labels_fg = Mat::ones(strawberryPts.size(), 1, CV_32SC1);

    for( unsigned i = 0; i < strawberryPts.size(); i++ )
    {
        //Label or descriptor: the hsv representation of the pixel
        Vec3b descriptor = img_hsv.at<Vec3b>(strawberryPts[i].y, strawberryPts[i].x);
        trainingDataForeground.at<float>(i, 0) = descriptor[0];
        trainingDataForeground.at<float>(i, 1) = descriptor[1];
        trainingDataForeground.at<float>(i, 2) = descriptor[2];
    }

    Mat trainingDataBackground(backgroundPts.size(), 3, CV_32FC1);
    //Label of a background pixel is 0
    Mat labels_bg = Mat::zeros(backgroundPts.size(), 1, CV_32SC1);

    for( unsigned i = 0; i < backgroundPts.size(); i++ )
    {
        //Label or descriptor: the hsv representation of the pixel
        Vec3b descriptor = img_hsv.at<Vec3b>(backgroundPts[i].y, backgroundPts[i].x);
        trainingDataBackground.at<float>(i, 0) = descriptor[0];
        trainingDataBackground.at<float>(i, 1) = descriptor[1];
        trainingDataBackground.at<float>(i, 2) = descriptor[2];
    }

    //Concatenate both vectors into 1 trainingData vector and 1 label vector
    Mat trainingData, labels;
    vconcat(trainingDataForeground, trainingDataBackground, trainingData);
    vconcat(labels_fg, labels_bg, labels);


    ///5.3: TRAIN A CLASSIFIER USING THE ABOVE DATA (KNN, NBC AND SVM)

    ///Train the K-Nearest-Neighbour classifier
    Ptr<KNearest> kNN = trainkNN(trainingData, labels);


    ///5.4: CLASSIFY ALL PIXELS OF THE IMAGE, CREATE A MASK
    classify(img_hsv, kNN);


    //Verbeteren? Groene kanaal wegflikkeren!
}
