/*
 * Copyright (C) 2019  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
using namespace cv;
using namespace std;



const int max_value_H = 180;
const int max_value = 255;


float stopSize{0.0};

int low_H = 0, low_S = 0, low_V = 0;
int high_H = 179, high_S = 255, high_V = 255;


bool stop = false;

//int counter = 0;

int stopCounter = 0;

bool byStop = false;
bool notRight = false;
bool notLeft = false;
bool notForward = false;


double angle( Point pt1, Point pt2, Point pt0 ) {
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


void find_squares(Mat& image, vector<vector<Point> >& squares)
{
    // blur will enhance edge detection
    Mat blurred(image);
    medianBlur(image, blurred, 9);

    Mat gray0(blurred.size(), CV_8U), gray;
    vector<vector<Point> > contours;

    // find squares in every color plane of the image (only threshold level in our case for blue square)
    for (int c = 0; c < 1; c++)
    {
        int ch[] = {c, 0};
        mixChannels(&blurred, 1, &gray0, 1, ch, 1);
        const int threshold_level = 2;
        for (int l = 0; l < threshold_level; l++)
        {
            // Use Canny instead of zero threshold level!
            // Canny helps to catch squares with gradient shading
            if (l == 0)
            {
                Canny(gray0, gray, 10, 20, 3); //

                // Dilate helps to remove potential holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                    gray = gray0 >= (l+1) * 255 / threshold_level;
            }

            // Find contours and store them in a list
            findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

            // Test contours
            vector<Point> approx;
            for (size_t i = 0; i < contours.size(); i++)
            {
                    // approximate contour with accuracy proportional
                    // to the contour perimeter
                    approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                    // Note: absolute value of an area is used because
                    // area may be positive or negative - in accordance with the
                    // contour orientation
                    //checks for amount of vector points, if == 4 (square) and its area is bigger than 1000 pixels and makes sure that the vectors aren't going through the square (goes for corners)

                    if (approx.size() > 3 && approx.size() < 9 &&
                            fabs(contourArea(Mat(approx))) > 1000 &&
                            isContourConvex(Mat(approx)))
                    {
                            double maxCosine = 0;

                            std::cout << "vectores of squares is: (" << approx.size() << ")" << std::endl;
                            //checks the angles between vectors to see if it's a reasonable angle, and if it's accepted, the square is added to the squares list.
                            for (int j = 2; j < 5; j++)
                            {
                                    double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                                    maxCosine = MAX(maxCosine, cosine);
                            }

                            if (maxCosine < 0.7)
                                    squares.push_back(approx);
                    }
            }
        }
    }
}

Mat debugSquares( vector<vector<Point> > squares, Mat image )
{
    for ( unsigned int i = 0; i< squares.size(); i++ ) {
        // draw contour
        drawContours(image, squares, i, Scalar(255,0,0), 1, 8, vector<Vec4i>(), 0, Point());

        // draw bounding rect around the square in pos i.
        Rect rect = boundingRect(Mat(squares[i]));
        rectangle(image, rect.tl(), rect.br(), Scalar(0,255,0), 2, 8, 0);

    }

    return image;
}

//this method corrects the car's placement depending on the car in front (pos is the centre of the acc car)
double correct_turn(vector<Point> squares)
{
    Rect rect = boundingRect(Mat(squares));
    double pos = rect.x+(rect.width/2);
    double offset = pos - 320;

    std::cout << "rect pos is: " << pos << std::endl;
   if( pos < 220)
    {
    std::cout << "turn hard left" << std::endl;
    }
   else if( pos < 290)
    {
    std::cout << "turn small left" << std::endl;
    }
    else if( pos > 420)
    {
    std::cout << "turn hard right" << std::endl;
    }
    else if( pos > 350)
    {
    std::cout << "turn small right" << std::endl;
    }
    else
    {
     std::cout << "no correction" << std::endl;   
    }
    return offset;
}


void find_stop(Mat& image, vector<vector<Point> >& stopSigns)
{
    // blur will enhance edge detection
    Mat blurred(image);
    medianBlur(image, blurred, 9);

    Mat gray0(blurred.size(), CV_8U), gray;
    vector<vector<Point> > contours;

    // find hexagons in every color plane of the image (only threshold level in our case for red hexagons)
    for (int c = 0; c < 1; c++)
    {
        int ch[] = {c, 0};
        mixChannels(&blurred, 1, &gray0, 1, ch, 1);
      
        const int threshold_level = 2;
        for (int l = 0; l < threshold_level; l++)
        {
            // Use Canny instead of zero threshold level!
            // Canny helps to catch hexagons with gradient shading
            if (l == 0)
            {
                Canny(gray0, gray, 10, 20, 3); //

                // Dilate helps to remove potential holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                    gray = gray0 >= (l+1) * 255 / threshold_level;
            }

            // Find contours and store them in a list
            findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

            // Test contours
            vector<Point> approx;
            for (size_t i = 0; i < contours.size(); i++)
            {
                    // approximate contour with accuracy proportional
                    // to the contour perimeter
                    approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                    // Note: absolute value of an area is used because
                    // area may be positive or negative - in accordance with the
                    // contour orientation
                    //checks for amount of vector points, if  amount of vectors are between 6-->8 (hexagon) and its area is bigger than 1000 pixels and makes sure that the vectors aren't going through the square (goes for corners)
                    //                                                              The reason for 6-->8 instead of only 6 is because of the imperfection of the hexagon making small vectors appear.
                    if (approx.size() > 5 && approx.size() < 9 &&
                            fabs(contourArea(Mat(approx))) > 1000 &&
                            isContourConvex(Mat(approx)))
                    {

                               // std::cout << "vectors of stopsign : (" << approx.size() << ")" << std::endl;

                            double maxCosine = 0;
                            for (int j = 2; j < 6; j++)
                            {
                                    double cosine = fabs(angle(approx[j%6], approx[j-2], approx[j-1]));
                                    maxCosine = MAX(maxCosine, cosine);
                            }
                            //if maxcosine is less than 1, it counts as a hexagon and is added to stopSigns.
                            if (maxCosine < 1)
                                    stopSigns.push_back(approx);
                               
                    }
            }
        }
    }    if (stopSigns.size() > 0)
                    {
                        std::cout << "found stop sign" << std::endl;
                        std::cout << contourArea(stopSigns[0]) << std::endl;
                    }

                //if stopsign is between the area 14000 and 155000, it means we are close enough to begin the stop sequence.

         if (stopSigns.size() > 0 && contourArea(stopSigns[0]) >3000)

         {
            //sets bystop boolean to true
           std::cout << "close to stop sign, start hardcoded sequence" << std::endl;
           byStop = true;

           stopSize = contourArea(stopSigns[0]);

         }

        else 
        {
        }
}




Mat debugStop(vector<vector<Point> > stopSigns, Mat image )
{
    for ( unsigned int i = 0; i< stopSigns.size(); i++ ) {
        // draw contour
        drawContours(image, stopSigns, i, Scalar(255,0,0), 1, 8, vector<Vec4i>(), 0, Point());

        // draw bounding rect around the stopsign
        Rect rect = boundingRect(Mat(stopSigns[i]));
        rectangle(image, rect.tl(), rect.br(), Scalar(0,255,0), 2, 8, 0);

    }

    return image;
}


void find_sign(Mat& image, vector<vector<Point> >& sign, int version )
{
    // blur will enhance edge detection
    Mat blurred(image);
    medianBlur(image, blurred, 9);

    Mat gray0(blurred.size(), CV_8U), gray;
    vector<vector<Point> > contours;

    // find triangles in every color plane of the image
    for (int c = 0; c < 1; c++)
    {
        int ch[] = {c, 0};
        mixChannels(&blurred, 1, &gray0, 1, ch, 1);
        // try several threshold levels
        const int threshold_level = 2;
        for (int l = 0; l < threshold_level; l++)
        {
            // Use Canny instead of zero threshold level!
            // Canny helps to catch squares with gradient shading
            if (l == 0)
            {
                Canny(gray0, gray, 10, 20, 3); //

                // Dilate helps to remove potential holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                    gray = gray0 >= (l+1) * 255 / threshold_level;
            }

            // Find contours and store them in a list
            findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

            // Test contours
            vector<Point> approx;
            for (size_t i = 0; i < contours.size(); i++)
            {
                    // approximate contour with accuracy proportional
                    // to the contour perimeter
                    approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                    // Note: absolute value of an area is used because
                    // area may be positive or negative - in accordance with the
                    // contour orientation
                     //checks for amount of vector points, if  amount of vectors are between 2-->8 (Triangle) and its area is bigger than 1000 pixels and makes sure that the vectors aren't going through the square (goes for corners)
                    //                                                              The reason for 2-->8 instead of only 3 is because of the imperfection of the triangle making small vectors appear.
                    // triangle will not be as sharp as preffered, which makes the corners a bit round, hence why the big window of vectors are allowed.
                    if (approx.size() > 2 && approx.size() < 9 &&
                            fabs(contourArea(Mat(approx))) > 1000 &&
                            isContourConvex(Mat(approx)))
                    {
                            double maxCosine = 0;

                            for (int j = 2; j < 4; j++)
                            {
                                    double cosine = fabs(angle(approx[j%3], approx[j-2], approx[j-1]));
                                    maxCosine = MAX(maxCosine, cosine);
                                   
                            }

                            // if masCosine is less than 1, it gets pushed to the sign list and notRight boolean is set to true.
                            if (maxCosine < 1)
                            {
                                sign.push_back(approx);

                                if(version == 1){
                                notRight = true;
                                std::cout << "sign 1: grön " << std::endl;
                                std::cout << "bool for green is : " << notRight << std::endl;
                                }
                                if(version == 2){
                                notLeft = true; 
                                std::cout <<"sign 2: rosa" << std::endl;
                                //std::cout << "vectors of pink : (" << approx.size() << ")" << std::endl;
                                //std::cout << "not allowed to turn left: " << std::endl;
                                }
                                if(version == 3){
                                notForward = true;
                                std::cout << "sign 3: lila " << std::endl;
                                //std::cout << "vectors of purple : (" << approx.size() << ")" << std::endl;
                                }
                            }
                    }
            }
        }
    }
}


Mat debugSign(vector<vector<Point> > sign, Mat image )
{
    for ( unsigned int i = 0; i< sign.size(); i++ ) {
        // draw contour
        drawContours(image, sign, i, Scalar(255,0,0), 1, 8, vector<Vec4i>(), 0, Point());

        // draw bounding rect around the detected sign.
        Rect rect = boundingRect(Mat(sign[i]));
        rectangle(image, rect.tl(), rect.br(), Scalar(0,255,0), 2, 8, 0);

    }

    return image;
}



int32_t main(int32_t argc, char **argv) {
    opendlv::proxy::sizeReading msg;
    opendlv::proxy::stopReading msg2;
    opendlv::proxy::signRec msg3;
    opendlv::proxy::correctTurn msg4;
    opendlv::proxy::stopRequest msg5;
    opendlv::proxy::startDistance msg6;

        float tempDistReading{0.0};

       float frontDistance{0.0};

       float sideDistance{0.0};


    int32_t retCode{1};
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);


    

    if ( (0 == commandlineArguments.count("cid")) ||
         (0 == commandlineArguments.count("name")) ||
         (0 == commandlineArguments.count("width")) ||
         (0 == commandlineArguments.count("height")) ) {
        std::cerr << argv[0] << " attaches to a shared memory area containing an ARGB image." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --cid=<OD4 session> --name=<name of shared memory area> [--verbose]" << std::endl;
        std::cerr << "         --cid:    CID of the OD4Session to send and receive messages" << std::endl;
        std::cerr << "         --name:   name of the shared memory area to attach" << std::endl;
        std::cerr << "         --width:  width of the frame" << std::endl;
        std::cerr << "         --height: height of the frame" << std::endl;
        std::cerr << "Example: " << argv[0] << " --cid=112 --name=img.i420 --width=640 --height=480" << std::endl;
    }
    else {
        const std::string NAME{commandlineArguments["name"]};
        const uint32_t WIDTH{static_cast<uint32_t>(std::stoi(commandlineArguments["width"]))};
        const uint32_t HEIGHT{static_cast<uint32_t>(std::stoi(commandlineArguments["height"]))};
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};



            cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};


//-------________-----_--__----_----____----___-----_-----____----______-----_--___--__----__---__----__---_-----

        auto onDistanceReading{[VERBOSE, &tempDistReading, &frontDistance, &sideDistance](cluon::data::Envelope &&envelope)

            // &<variables> will be captured by reference (instead of value only)

            {

                auto msg = cluon::extractMessage<opendlv::proxy::DistanceReading>(std::move(envelope));

                const uint16_t senderStamp = envelope.senderStamp(); // Local variables are not available outside the lambda function

                tempDistReading = msg.distance(); // Corresponds to odvd message set



                    if (VERBOSE){

                        //std::cout << "prints if we are in the find distance" << std::endl;


                    }

                    if(senderStamp == 0){

                        frontDistance = tempDistReading;

                    }else if(senderStamp == 1){

                        sideDistance = tempDistReading;

                    }





            }

        };

        od4.dataTrigger(opendlv::proxy::DistanceReading::ID(), onDistanceReading);

//-------___________---__-----------_____----__________---_-------__----_---_--__------__---------------_-------_--------


        // Attach to the shared memory.
        std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
        if (sharedMemory && sharedMemory->valid()) {
            std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << " (" << sharedMemory->size() << " bytes)." << std::endl;

            // Interface to a running OpenDaVINCI session; here, you can send and receive messages.

            // Endless loop; end the program by pressing Ctrl-C.
            while (od4.isRunning()) {
                cv::Mat img;

                // Wait for a notification of a new frame.
                sharedMemory->wait();

                // Lock the shared memory.
                sharedMemory->lock();
                {
                    // Copy image into cvMat structure.
                    // Be aware of that any code between lock/unlock is blocking
                    // the camera to provide the next frame. Thus, any
                    // computationally heavy algorithms should be placed outside
                    // lock/unlock.
                    cv::Mat wrapped(HEIGHT, WIDTH, CV_8UC4, sharedMemory->data());
                    img = wrapped.clone();
                }
                sharedMemory->unlock();

             
                
                // TODO: Do something with the frame.
                //declaring hsv, hsv threshold frames.
                cv::Mat frame_HSV;
                cv::Mat frame_threshold, stop_threshold, sign1_threshold, sign2_threshold, sign3_threshold, cars_threshold;//, stop_threshold;
                //different lists for squares (acc car), stopsign and signs for turning rules
                vector<vector<Point> > squares;
                vector<vector<Point> > stopSigns;
                vector<vector<Point> > sign;
                vector<vector<Point> > cars;

               
                //converts the image that we copied from shared memory to HSV and save it as an HSV image called frame_HSV
                cvtColor(img, frame_HSV, COLOR_BGR2HSV);
                
                //sets the Hue/Saturation/Value for the different thresholds depending on if it's looking for car/stopsign/signs. (stop is red, car is blue and signs are green)
                inRange(frame_HSV, Scalar(170, 175, 35), Scalar(180, 255, 180), stop_threshold);   // Stop sign (red)
                inRange(frame_HSV, Scalar(68, 148, 80), Scalar(116, 208, 126), frame_threshold);  // Acc car (dark blue)
                inRange(frame_HSV, Scalar(52, 97, 78), Scalar(88, 181, 125), sign1_threshold);   //Green
                inRange(frame_HSV, Scalar(154, 135, 113), Scalar(170, 224, 176), sign2_threshold);//rosa
                inRange(frame_HSV, Scalar(114, 111, 102), Scalar(134, 183, 169), sign3_threshold);//Yellow
                
                


                //calls on the find methods with the threshold frames and the list which the object will be saved in
                find_squares(frame_threshold, squares);
                find_stop(stop_threshold, stopSigns);
                find_sign(sign1_threshold, sign, 1);
                find_sign(sign2_threshold, sign, 2);
                find_sign(sign3_threshold, sign, 3);

                
              
                // Display image.
                if (VERBOSE) {


                    if(notRight == true || notForward == true || notLeft == true){                    

                        msg3.rightSign(notRight);

                        msg3.leftSign(notLeft);

                        msg3.straightSign(notForward);

                        od4.send(msg3);

                        std::cout << "bool for green is : " << notRight << std::endl;


                        std::cout << "sending sign stuff" << std::endl;

                   }
                  

                    //counts amount of cars in intersection  s
                 

                    // if blue square(car) is detected and byStop is not true, sets speed depending on size of rectangle (distance to other car)
                   if (squares.size() > 0 && byStop == false && stop == false)
                    {
                        float carSize = contourArea(squares[0]);

                        std::cout << "found square " << std::endl;
                        std::cout << carSize << std::endl;

                        double offset = correct_turn(squares[0]);


                        msg4.offset(offset);

                        od4.send(msg4);

                        stopCounter = 0;

                        
                    }

                    //if byStop is true (we are by the stopsign), we stop following the car in front of us.
                     else if (byStop == true)
                    {
                     bool stopseq = true;

                     std::cout << "sending stopsign bajs" << std::endl;



                        msg2.stop(stopseq);

                        msg2.signSize(stopSize);

                        od4.send(msg2);
                        

                        //counter += 1;
                    }else if (stop == true){

                        

                        
                        std::cout << "sending to object" << std::endl;



                        msg6.frontDistance(frontDistance);
                        msg6.sideDistance(sideDistance);
                        od4.send(msg6);

                    }

                   

                    //if car infront is not detected, stop pedal.
                    else if(stopCounter < 3)
                    {
                        std::cout <<"no square found, stop car " << std::endl;

                        msg5.stopping("stop");

                        od4.send(msg5);

                        stopCounter += 1;

                    }

                    //clears any remaining squares just in case they would linger.
                    squares.clear();



                    cv::waitKey(1);

//----------____________----_____---__------______---_----____----_--__---______---__------__---_----------_----------------__---_-------_


 


/*
       float tempDistReading{0.0};
       float frontDistance{0.0};
       float sideDistance{0.0};
       
        auto onDistanceReading{[VERBOSE, &tempDistReading, &frontDistance, &sideDistance](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {
                auto msg = cluon::extractMessage<opendlv::proxy::DistanceReading>(std::move(envelope));
                const uint16_t senderStamp = envelope.senderStamp(); // Local variables are not available outside the lambda function
                tempDistReading = msg.distance(); // Corresponds to odvd message set

                

                    if (VERBOSE){

                    }


                    if(senderStamp == 0){

                        frontDistance = tempDistReading;

                    }else if(senderStamp == 1){

                        sideDistance = tempDistReading;

                    }

                
                
            }
        };

        od4.dataTrigger(opendlv::proxy::DistanceReading::ID(), onDistanceReading);

//------------_____________----_----_--__________------_____---_________-----_________________-----__---__----_----____----_-------__------------_--

       

       //carCounter is just a placeholder for the real queCounter from counting cars.
       int carCounter = 3;
       int carCheck = 0;
       bool tempTrigger = false;
        auto carQueue{[VERBOSE, &frontDistance, &sideDistance, &carCounter, &carCheck, &tempTrigger](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {
                auto msg7 = cluon::extractMessage<opendlv::proxy::stopDone>(std::move(envelope));
                
            
                     tempTrigger = msg7.done();  

                    if (VERBOSE){

                    if(frontDistance > 0.05 && frontDistance < 0.25){

                        carCheck += 1;

                    }else if(sideDistance > 0.05 && sideDistance < 0.15){

                        carCheck += 1;

                    }else{

                        carCheck = 0;

                    }

                    if(carCheck == 3){

                        carCounter -= 1;

                        carCheck = 0;
                    }

                    

                    if(carCounter == 0){

                        std::cout << "time to vroomvrooom" << std::endl;

                    }

                }
                
            }
        };

        od4.dataTrigger(opendlv::proxy::stopDone::ID(), carQueue);
 */

//-------_____________---___---_----___-----____-----_----_----_--___---__---___---___--___---__---_---------------_--_________-----_-------__----_-                    

        

        auto stopDone{[VERBOSE, &byStop](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {
                auto msg9 = cluon::extractMessage<opendlv::proxy::stopDone>(std::move(envelope));
                
                 // Corresponds to odvd message set

                stop = true;
                
                byStop = msg9.done();  


                
                
            }
        };
        od4.dataTrigger(opendlv::proxy::stopDone::ID(), stopDone);

//-------________-----_--__------_--------__---__----_-------_----_-----------------


//-------_____---__---__----_-___---___--__----____________----___--__----__---_______----__----__---_-----_---------_____----__--------__---        

                }
            }
        }






//----------____________----_----______---_____-----___---________---__---__---__----____----_-------_----_-------_----_--__-

/*

        auto onDistanceReading{[VERBOSE, &tempDistReading](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {
                auto msg = cluon::extractMessage<opendlv::proxy::DistanceReading>(std::move(envelope));
                const uint16_t senderStamp = envelope.senderStamp(); // Local variables are not available outside the lambda function
                tempDistReading = msg.distance(); // Corresponds to odvd message set
              
                if(senderStamp == 0){

                    if (VERBOSE)
                {
                    frontDistance = tempDistReading;
                }

                }


            }
        };
        od4.dataTrigger(opendlv::proxy::DistanceReading::ID(), onDistanceReading);

*/

//-------____________----__---------___--______--_____---______-----_____---________------_____---__----_----__---_-------__----_        


        retCode = 0;
    }
    return retCode;
}



//compile cluon msc locally
//write in cmakelist hardcoded
//comment out 