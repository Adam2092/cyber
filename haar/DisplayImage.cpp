#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
using namespace std;
using namespace cv;
void detectAndDisplay( Mat frame );
CascadeClassifier cascade;
int main( int argc, const char** argv )
{
    CommandLineParser parser(argc, argv,
                             "{help h||}"
                             "{cascade|../cascade/car-28.xml|Path to face cascade.}");
    parser.about( "\nThis program demonstrates using the cv::CascadeClassifier class to detect cars in a video stream. You can use Haar or LBP features.\n\n" );
    parser.printMessage();
    String face_cascade_name = parser.get<String>("cascade");
    //-- 1. Load the cascades
    if( !cascade.load( face_cascade_name ) )
    {
        cout << "--(!)Error loading cascade\n";
        return -1;
    };
    VideoCapture capture;
    //-- 2. Read the video stream
    //capture.open("../recordings/new-19-04-29/car-360-light.mp4"  );
    capture.open(0);
    Mat frame;
    while ( capture.read(frame) )
    {
        if( frame.empty() )
        {
            cout << "--(!) No captured frame -- Break!\n";
            break;
        }
        //-- 3. Apply the classifier to the frame
        detectAndDisplay( frame );
        if( waitKey(30) == 27 )
        {
            break; // escape
        }
    }
    return 0;
}
void detectAndDisplay( Mat frame )
{
    Mat frame_gray;
    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );
    //-- Detect faces
    std::vector<Rect> cars;
    cascade.detectMultiScale( frame_gray, cars, 1.1, 4 );
    for (vector<Rect>::const_iterator r = cars.begin(); r != cars.end(); r++)
	rectangle(frame, *r, Scalar(0,0,255), 2, 8,0);
    //-- Show what you got
    imshow( "Capture - Face detection", frame );
}
