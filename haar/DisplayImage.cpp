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
                             "{cascade|cascade.xml|Path to face cascade.}");
    parser.about( "\nThis program demonstrates using the cv::CascadeClassifier class to detect objects (Face + eyes) in a video stream. You can use Haar or LBP features.\n\n" );
    parser.printMessage();
    String face_cascade_name = parser.get<String>("./cascade/car");
    //-- 1. Load the cascades
    if( !cascade.load( face_cascade_name ) )
    {
        cout << "--(!)Error loading cascade\n";
        return -1;
    };
    VideoCapture capture;
    //-- 2. Read the video stream
    capture.open("./recordings/rotate/pos/mp4/car-360-light-close.mp4"  );
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
        if( waitKey(10) == 27 )
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
    cascade.detectMultiScale( frame_gray, cars, 1.05, 3, 6 );
    for ( size_t i = 0; i < cars.size(); i++ )
    {
        Point center( cars[i].x + cars[i].width/2, cars[i].y + cars[i].height/2 );
        ellipse( frame, center, Size( cars[i].width/2, cars[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 255 ), 4 );
        Mat faceROI = frame_gray( cars[i] );
    }
    //-- Show what you got
    imshow( "Capture - Face detection", frame );
}
