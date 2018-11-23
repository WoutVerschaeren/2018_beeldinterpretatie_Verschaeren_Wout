#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

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
    Ptr<Feature2D> detector;
    string method;
    switch(keypointType)
    {
        case 1 :
            {
                Ptr<ORB> detector_ORB = ORB::create(500);
                detector = detector_ORB;
                method = "ORB";
            }
            break;
        case 2 :
            {
                Ptr<BRISK> detector_BRISK = BRISK::create(500);
                detector = detector_BRISK;
                method = "BRISK";
            }
            break;
        case 3 :
            {
                Ptr<AKAZE> detector_AKAZE = AKAZE::create(500);
                detector = detector_AKAZE;
                method = "AKAZE";
            }
            break;
    }

    //Detect keypoints
    vector<KeyPoint> keypointsObj, keypointsImg;
    detector->detect(obj.clone(), keypointsObj);
    detector->detect(img.clone(), keypointsImg);

    //Draw keypoints
    Mat img_keypointsObj, img_keypointsImg;
    drawKeypoints(obj.clone(), keypointsObj, img_keypointsObj, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    drawKeypoints(img.clone(), keypointsImg, img_keypointsImg, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

    string titleT = "Template with keypoints: " + method;
    string titleI = "Image with keypoints: " + method;
    imshow(titleT, img_keypointsObj);   waitKey(0);
    imshow(titleI, img_keypointsImg);   waitKey(0);

    return 0;
}

///Detect features, compute the descriptors and match using ORB (1), BRISK (2), or AKAZE (3)
/*
    obj_loc: location of the template image
    img_loc: location of the image itself
    keypointType: keypoint type: ORB (1), BRISK (2), or AKAZE (3)
*/
int detectAndMatch(string obj_loc, string img_loc, int keypointType)
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
    Ptr<Feature2D> detector;
    string method;
    switch(keypointType)
    {
        case 1 :
            {
                Ptr<ORB> detector_ORB = ORB::create(500);
                detector = detector_ORB;
                method = "ORB";
            }
            break;
        case 2 :
            {
                Ptr<BRISK> detector_BRISK = BRISK::create(500);
                detector = detector_BRISK;
                method = "BRISK";
            }
            break;
        case 3 :
            {
                Ptr<AKAZE> detector_AKAZE = AKAZE::create(500);
                detector = detector_AKAZE;
                method = "AKAZE";
            }
            break;
    }

    //Detect features and compute their descriptors
    vector<KeyPoint> keypointsObj, keypointsImg;
    Mat descriptorsObj, descriptorsImg;
    detector->detectAndCompute(obj.clone(), Mat(), keypointsObj, descriptorsObj);
    detector->detectAndCompute(img.clone(), Mat(), keypointsImg, descriptorsImg);

    ///Match the two images using the computed descriptors
    //Brute force matching
    BFMatcher matcher(NORM_L2);
    vector<DMatch> matches;
    matcher.match(descriptorsObj, descriptorsImg, matches);

    //Draw the matches
    Mat img_matches;
    drawMatches(obj, keypointsObj, img, keypointsImg, matches, img_matches);
    //Show the matches
    string title = "Detected matches: " + method;
    imshow(title, img_matches);    waitKey(0);
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

    ///Detect features, compute the descriptors and match using ORB (1), BRISK (2), or AKAZE (3)
    detectAndMatch(kb_obj_loc, kb_img_loc, 1);

    ///4.3
    /*
    double max_dist = 0;
    double min_dist = 0;
    ///Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_1.rows; i++)
    {
        double dist = matches[i].distance;
        if(dist < min_dist) min_dist = dist;
        if(dist > max_dist) max_dist = dist;
    }

    ///Draw only "good" matches (those whose distance is less than 3*min_dist
    vector<DMatch> good_matches;
    for(int i = 0; i < descriptors_1.rows; i++)
    {
        if(matches[i].distance <= 3*min_dist)
        {
            good_matches.push_back(matches[i]);
        }
    }
    Mat img_matches;
    drawMatches(img_1, keypoints_1, img_2, keypoints_2, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), );

    */

    //Mat H = findHomography(obj, scene, RANSAC);
}
