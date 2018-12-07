#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>

using namespace std;
using namespace cv;
using namespace cv::ml;

vector<Point2f> strawberryPts;
vector<Point2f> backgroundPts;

//Using global variables was the easiest way of not having to copy/paste code
Ptr<KNearest> kNN;
Ptr<NormalBayesClassifier> NBC;
Ptr<SVM> SVMach;

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

///Open and close an image
Mat openAndClose(Mat img)
{
    //Opening
    erode(img, img, Mat(), Point(-1,-1), 2);
    dilate(img, img, Mat(), Point(-1,-1), 2);

    //Closing
    dilate(img, img, Mat(), Point(-1,-1), 5);
    erode(img, img, Mat(), Point(-1,-1), 5);

    return img;
}

///Mask an image
Mat maskImage(Mat img, Mat mask)
{
    //Split the image into BGR channels
    vector<Mat> channels;
    split(img, channels);                           //Split the colour image into three different channels

    //Apply the mask to every channel
    vector<Mat> channels_masked = channels;
    channels_masked[0] = channels[0] & mask;
    channels_masked[1] = channels[1] & mask;
    channels_masked[2] = channels[2] & mask;

    //Merge the masked channels back into one image
    Mat masked_img(img.rows, img.cols, CV_8UC3);    //Create a 3 channel image
    merge(channels_masked,masked_img);

    return masked_img;
}

///Delete the green channel of an image
Mat deleteGreen(Mat img)
{
    //Split the image into BGR channels
    vector<Mat> channels;
    split(img, channels);                           //Split the colour image into three different channels

    //Delete the green channel
    channels[1] = 0;

    //Merge the masked channels back into one image
    Mat img_noGreen(img.rows, img.cols, CV_8UC3);    //Create a 3 channel image
    merge(channels,img_noGreen);

    return img_noGreen;
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
    //https://stackoverflow.com/a/30987458
    kNN = KNearest::create();
    Ptr<TrainData> trainingDataKNN = TrainData::create(trainingData, SampleTypes::ROW_SAMPLE, labels);

    kNN->setIsClassifier(true);
    kNN->setAlgorithmType(KNearest::Types::BRUTE_FORCE);
    kNN->setDefaultK(3);

    kNN->train(trainingDataKNN);

    return kNN;
}

///Train a Normal Bayes classifier
/*
    trainingData: vector containing all the training points
    labels: vector containing the descriptors of the training points
*/
Ptr<NormalBayesClassifier> trainBayes(Mat trainingData, Mat labels)
{
    NBC = NormalBayesClassifier::create();
    Ptr<TrainData> trainingDataNBC = TrainData::create(trainingData, SampleTypes::ROW_SAMPLE, labels);

    NBC->train(trainingDataNBC);

    return NBC;
}

///Train a Support Vector Machine
/*
    trainingData: vector containing all the training points
    labels: vector containing the descriptors of the training points
*/
Ptr<SVM> trainSVM(Mat trainingData, Mat labels)
{
    //https://docs.opencv.org/master/d1/d73/tutorial_introduction_to_svm.html
    SVMach = SVM::create();
    Ptr<TrainData> trainingDataSVM = TrainData::create(trainingData, SampleTypes::ROW_SAMPLE, labels);

    SVMach->setType(SVM::C_SVC);
    SVMach->setKernel(SVM::LINEAR);
    SVMach->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));

    SVMach->train(trainingDataSVM);

    return SVMach;
}


///Classify an image according to the trained classifier, return a mask
/*
    img: an image
    classifierType: 0: Nearest-Neighbour
                    1: Normal Bayes Classifier
                    2: Support Vector Machine
*/
Mat classify(Mat img, int classifierType)
{
    Mat mask = Mat::zeros(img.rows, img.cols, CV_8UC1);
    Mat result;

    ///Loop through the image pixels, classify and create a mask of the result
    for ( int i = 0; i < img.rows; i++ )
    {
        ///Per row, create a vector (pixelRow) that stores the H, S and V channels of the pixels (currPixel) on that row
        //Create the vector to store pixels
        Mat pixelRow(img.cols, 3, CV_32FC1);
        //Loop through a row
        for ( int j = 0; j < img.cols; j++ )
        {
            Vec3b currPixel = img.at<Vec3b>(i, j);

            pixelRow.at<float>(j, 0) = currPixel[0];
            pixelRow.at<float>(j, 1) = currPixel[1];
            pixelRow.at<float>(j, 2) = currPixel[2];
        }

        ///Classify this row of pixels, store the results
        switch(classifierType)
        {
            case 0 :
            {
                kNN->findNearest(pixelRow, kNN->getDefaultK(), result);
                break;
            }

            case 1 :
            {
                NBC->predict(pixelRow, result);
                break;
            }

            case 2 :
            {
                SVMach->predict(pixelRow, result);
                break;
            }
        }

        ///Loop through the row again, open the mask if the pixel is classified as a strawberry
        for ( int j = 0; j < img.cols; j++ )
        {
            if ( result.at<float>(j) != 0)
            {
                mask.at<uchar>(i, j) = 255;
            }
        }
    }

    return mask;
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

    //Delete the green channel of the image
    Mat img_noGreen = deleteGreen(img);
    cvtColor(img_noGreen, img_hsv, COLOR_BGR2HSV);

    //Blur the image a bit so the clicked points arent using the black and green dots on the strawberry
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
    trainkNN(trainingData, labels);
    trainBayes(trainingData, labels);
    trainSVM(trainingData, labels);


    ///5.4: CLASSIFY ALL PIXELS OF THE IMAGE, CREATE A MASK

    ///Create masks based on the classification of the image
    /*
        0: Nearest-Neighbour
        1: Normal Bayes Classifier
        2: Support Vector Machine
    */
    Mat maskKNN = classify(img_hsv, 0);
    Mat maskNBC = classify(img_hsv, 1);
    Mat maskSVM = classify(img_hsv, 2);

    ///Clean the masks
    maskKNN = openAndClose(maskKNN);
    maskNBC = openAndClose(maskNBC);
    maskSVM = openAndClose(maskSVM);

    ///Mask the image
    img = maskImage(img, maskKNN);
    imshow("Masked image: kNN", img);  waitKey(0);

    img = maskImage(img, maskNBC);
    imshow("Masked image: NBC", img);  waitKey(0);

    img = maskImage(img, maskSVM);
    imshow("Masked image: SVM", img);  waitKey(0);

    //Verbeteren? Groene kanaal wegflikkeren!

    return 0;
}
