#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

///Detect keypoints in the image
/*
    img: the image in which keypoints need to be detected
    keypointType: keypoint type: ORB (1), BRISK (2), or AKAZE (3)
*/
vector<KeyPoint> KeyPointDetection(Mat img, int keypointType)
{
    /// Detect features using ORB, BRISK, or AKAZE
    Ptr<Feature2D> detector;
    switch(keypointType)
    {
        case 1 :
            {
                Ptr<ORB> detector_ORB = ORB::create();
                detector = detector_ORB;
            }
            break;
        case 2 :
            {
                Ptr<BRISK> detector_BRISK = BRISK::create();
                detector = detector_BRISK;
            }
            break;
        case 3 :
            {
                Ptr<AKAZE> detector_AKAZE = AKAZE::create();
                detector = detector_AKAZE;
            }
            break;
    }

    //Detect keypoints
    vector<KeyPoint> keypointsImg;
    detector->detect(img.clone(), keypointsImg);

    return keypointsImg;
}

///Calculate descriptors for a given image and its keypoints
/*
    img: the image for which descriptors need to be calculated
    keypoints: the detected keypoints in the image
    keypointType: keypoint type: ORB (1), BRISK (2), or AKAZE (3)
*/
Mat KeyPointDescription(Mat img, vector<KeyPoint> keypoints, int keyPointType)
{
    /// Detect features using ORB, BRISK, or AKAZE
    Ptr<Feature2D> descriptor;
    switch(keyPointType)
    {
        case 1 :
            {
                Ptr<ORB> descriptor_ORB = ORB::create();
                descriptor = descriptor_ORB;
            }
            break;
        case 2 :
            {
                Ptr<BRISK> descriptor_BRISK = BRISK::create();
                descriptor = descriptor_BRISK;
            }
            break;
        case 3 :
            {
                Ptr<AKAZE> descriptor_AKAZE = AKAZE::create();
                descriptor = descriptor_AKAZE;
            }
            break;
    }

    //Detect features and compute their descriptors
    vector<KeyPoint> keypointsImg;
    Mat descriptorsImg;
    //descriptor->detectAndCompute(obj.clone(), Mat(), keypointsObj, descriptorsObj);
    descriptor->compute(img.clone(), keypoints, descriptorsImg);

    return descriptorsImg;
}


///Detect and draw features using ORB (1), BRISK (2), and AKAZE (3)
/*
    obj_loc: location of the template image
    img_loc: location of the image itself
    keypointType: keypoint type: ORB (1), BRISK (2), or AKAZE (3)
*/
int detectKeypoints(string obj_loc, string img_loc, int keypointType)
{
    ///Read the object and image
    Mat obj = imread(obj_loc);
    Mat img = imread(img_loc);
    //Check if the images can be found
    if ( obj.empty() || img.empty() ){
        cerr << "One or more images can't be found.";
        return -1;
    }

    /// Detect features using ORB, BRISK, or AKAZE
    string method;
    switch(keypointType)
    {
        case 1 :
            method = "ORB";
            break;
        case 2 :
            method = "BRISK";
            break;
        case 3 :
            method = "AKAZE";
            break;
    }

    //Detect keypoints
    vector<KeyPoint> keypointsObj, keypointsImg;
    keypointsObj = KeyPointDetection(obj.clone(), keypointType);
    keypointsImg = KeyPointDetection(img.clone(), keypointType);

    //Draw keypoints
    Mat img_keypointsObj, img_keypointsImg;
    drawKeypoints(obj.clone(), keypointsObj, img_keypointsObj, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    drawKeypoints(img.clone(), keypointsImg, img_keypointsImg, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

    /*
    string titleT = "Template with keypoints: " + method;
    string titleI = "Image with keypoints: " + method;
    imshow(titleT, img_keypointsObj);   waitKey(0);
    imshow(titleI, img_keypointsImg);   waitKey(0);
    */

    return 0;
}

///Detect features, compute the descriptors and match using ORB (1), BRISK (2), or AKAZE (3)
/*
    obj_loc: location of the template image
    img_loc: location of the image itself
    keypointType: keypoint type: ORB (1), BRISK (2), or AKAZE (3)
    homogr: whether or not the function is being used for a homography
    name: name of the object (used for titles)
*/
int detectAndMatch(string obj_loc, string img_loc, int keypointType, bool homogr, string name)
{
    ///Read the object and image
    Mat obj = imread(obj_loc);
    Mat img = imread(img_loc);
    //Check if the images can be found
    if ( obj.empty() || img.empty() ){
        cerr << "One or more images can't be found.";
        return -1;
    }

    /// Detect features using ORB, BRISK, or AKAZE
    string method;
    switch(keypointType)
    {
        case 1 :
            method = "ORB";
            break;
        case 2 :
            method = "BRISK";
            break;
        case 3 :
            method = "AKAZE";
            break;
    }

    //Detect keypoints
    vector<KeyPoint> keypointsObj, keypointsImg;
    keypointsObj = KeyPointDetection(obj.clone(), keypointType);
    keypointsImg = KeyPointDetection(img.clone(), keypointType);

    //Calculate descriptors using the detected keypoints
    Mat descriptorsObj, descriptorsImg;
    descriptorsObj = KeyPointDescription(obj.clone(), keypointsObj, keypointType);
    descriptorsImg = KeyPointDescription(img.clone(), keypointsImg, keypointType);

    ///Match the two images using the computed descriptors
    //Brute force matching
    BFMatcher matcher(NORM_L2);
    vector<DMatch> matches;
    matcher.match(descriptorsObj, descriptorsImg, matches);

    if ( homogr == 1 )
    {
        double max_dist = 0;
        double min_dist = 1000000;
        ///Quick calculation of max and min distances between keypoints
        for( int i = 0; i < matches.size(); i++)
        {
            double dist = matches[i].distance;
            if(dist < min_dist) min_dist = dist;
            if(dist > max_dist) max_dist = dist;
        }

        ///Draw only "good" matches (those whose distance is less than 3*min_dist)
        vector<DMatch> good_matches;
        for(int i = 0; i < matches.size(); i++)
        {
            if(matches[i].distance <= 3*min_dist)
            {
                good_matches.push_back(matches[i]);
            }
        }

        //Draw the good matches
        Mat img_matches;
        drawMatches(obj, keypointsObj, img, keypointsImg, good_matches, img_matches);

        //Show the matches
        string title = name + ": Good detected matches: " + method;
        imshow(title, img_matches);    waitKey(0);

        ///Use RANSAC to calculate a model that represents the transformation between the object and the image
        //Keep only the keypoints that had sufficiently low distances
        vector<Point2f> objGood;
        vector<Point2f> imgGood;

        for(int i=0; i < good_matches.size(); i++)
        {
            objGood.push_back(keypointsObj[good_matches[i].queryIdx].pt);
            imgGood.push_back(keypointsImg[good_matches[i].trainIdx].pt);
        }

        //Calculate homography
        Mat H = findHomography(objGood, imgGood, RANSAC);

        ///Calculate 4 points at the corners of the object, transform them into the axial system of the detected object, draw lines between them
        //Find the corners of the object in the object image
        vector<Point2f> objCorners(4);
        objCorners[0] = cvPoint(0,0);
        objCorners[1] = cvPoint(obj.cols, 0 );
        objCorners[2] = cvPoint(obj.cols, obj.rows);
        objCorners[3] = cvPoint(0, obj.rows);

        //Transform the object into the same axial system as the detected object
        vector<Point2f> imgCorners(4);
        perspectiveTransform(objCorners, imgCorners, H);

        //Draw lines between the corner points
            //Add the offset because the original object image has been placed to the left of the image, compensate for this
        Point2f offset = Point2f(obj.cols, 0);
        line(img_matches, ( imgCorners[0] + offset ), ( imgCorners[1] + offset ), Scalar::all(255), 3);
        line(img_matches, ( imgCorners[1] + offset ), ( imgCorners[2] + offset ), Scalar::all(255), 3);
        line(img_matches, ( imgCorners[2] + offset ), ( imgCorners[3] + offset ), Scalar::all(255), 3);
        line(img_matches, ( imgCorners[3] + offset ), ( imgCorners[0] + offset ), Scalar::all(255), 3);

        //Show the end result
        title = name + ": Final result, using " + method;
        imshow(title, img_matches);    waitKey(0);
    }
    else
    {
        //Draw the matches
        Mat img_matches;
        drawMatches(obj, keypointsObj, img, keypointsImg, matches, img_matches);
        //Show the matches
        string title = name + ": All detected matches: " + method;
        imshow(title, img_matches);    waitKey(0);
    }

    return 0;
}


int main(int argc, const char** argv)
{
    ///Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{help h usage ?   |      | print this message   }"
        "{kb_obj p1 |      | <required> path to the kinderbueno object }"
        "{kb_img p2 |      | <required> path to the kinderbueno image  }"
        "{fcf_obj p3|      | <required> path to the fitness cornflakes object  }"
        "{fcf_img p4|      | <required> path to the fitness cornflakes image   }"
    );

    if (parser.has("help"))
    {
        parser.printMessage();
        cerr << "TIP: Use absolute paths for the arguments." << endl;
        return 0;
    }

    ///Collect data from arguments
    string kb_obj_loc(parser.get<string>("kb_obj"));
    string kb_img_loc(parser.get<string>("kb_img"));
    string fcf_obj_loc(parser.get<string>("fcf_obj"));
    string fcf_img_loc(parser.get<string>("fcf_img"));
    //Check of de argumenten niet leeg zijn
    if ( kb_obj_loc.empty() || kb_img_loc.empty() || fcf_obj_loc.empty() || fcf_img_loc.empty() ){
        cerr << "There's something wrong with your arguments." << endl;
        parser.printMessage();
        return -1;
    }

    ///4.1: KEYPOINT DETECTION

    ///Detect and draw features using ORB (1), BRISK (2), and AKAZE (3)
    detectKeypoints(kb_obj_loc, kb_img_loc, 1);
    detectKeypoints(kb_obj_loc, kb_img_loc, 2);
    detectKeypoints(kb_obj_loc, kb_img_loc, 3);


    ///4.2: KEYPOINT DETECTION, DESCRIPTOR COMPUTATION, AND MATCHING

    ///Detect features, compute the descriptors and match using ORB (1), BRISK (2), or AKAZE (3), don't serach for homography (0)
    detectAndMatch(kb_obj_loc, kb_img_loc, 1, 0, "Bueno");


    ///4.3: ONLY KEEP GOOD MATCHES, CALCULATE HOMOGRAPHY

    ///Detect features, compute the descriptors match using ORB (1), BRISK (2), or AKAZE (3) and find the homography (1)
    detectAndMatch(kb_obj_loc, kb_img_loc, 1, 1, "Bueno");
    detectAndMatch(fcf_obj_loc, fcf_img_loc, 1, 1, "Cornflakes");
}
