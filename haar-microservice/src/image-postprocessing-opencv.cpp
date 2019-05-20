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
#include "opencv2/objdetect.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
using namespace std;
using namespace cv;
CascadeClassifier cascade;
static int carLeft = 0;
static int carForward = 0;
static int carRight = 0;
static int queue = 0;
static int middleCarCounter = 9
boolean countUp = true;
middleCarGone = false;
boolean carDetected = false;
void detectAndDisplay( Mat frame )
{
    std::vector<Rect> cars;
    Mat frame_gray;
    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );
    //-- Detect faces
    cascade.detectMultiScale( frame_gray, cars, 1.1, 4, 0, Size(100,100));
    if(cars.size() != 0){
    for (vector<Rect>::const_iterator r = cars.begin(); r != cars.end(); r++)
    {
    Rect rect = *r;
	rectangle(frame, *r, Scalar(0,0,255), 2, 8,0);
    double carpos = rect.x;
    //if a car is detected to the left and no previous car is detected, add 1 to carLeft
        if (carpos < 100 && carLeft < 1 && carForward < 1)
        {
        carLeft ++;
        }
        //if a car is detected in the middle and no previous car is detected, add 1 to carForward
        if (carpos > 100 && carpos < 320 && carForward < 1)
        {
        carForward ++;
        }
         if (carpos > 320 && carRight < 1)
        {
        carRight ++;
        }
    }
}
}

void detectAndCountdown( Mat frame ){
  std::vector<Rect> cars;
  Mat frame_gray;
  cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
  equalizeHist( frame_gray, frame_gray );
  //-- Detect faces
  cascade.detectMultiScale( frame_gray, cars, 1.1, 4, 0, Size(100,100));
  if(cars.size() != 0){
  for (vector<Rect>::const_iterator r = cars.begin(); r != cars.end(); r++)
  {
  Rect rect = *r;
  rectangle(frame, *r, Scalar(0,0,255), 2, 8,0);
  double carpos = rect.br().x;
  if(carpos > 500)
  {
    carDetected = true;
  }
}
}


int32_t main(int32_t argc, char **argv) {
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

        // Attach to the shared memory.
        std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
        if (sharedMemory && sharedMemory->valid()) {
            std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << " (" << sharedMemory->size() << " bytes)." << std::endl;

            // Interface to a running OpenDaVINCI session; here, you can send and receive messages.
            cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};
            // Endless loop; end the program by pressing Ctrl-C.

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

        auto carQueue{[VERBOSE, &frontDistance, &sideDistance, &queue, &carDetected](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {
                auto msg7 = cluon::extractMessage<opendlv::proxy::stopDone>(std::move(envelope));
                     tempTrigger = msg7.done();
                     const int16_t delay{500};
                    if(frontDistance > 0.05 && frontDistance < 0.25){
                        queue--;
                        std::this_thread::sleep_for(std::chrono::milliseconds(6 * delay));
                    }if(sideDistance > 0.05 && sideDistance < 0.15){
                        queue--;
                        std::this_thread::sleep_for(std::chrono::milliseconds(6 * delay));

                    }

                    if (carDetected)
                    queue--;
                    carDetected = false;
                    std::this_thread::sleep_for(std::chrono::milliseconds(6 * delay));

                }
            };

        od4.dataTrigger(opendlv::proxy::DistanceReading::ID(), carQueue);

        auto reachedStop{[&od4, &countUp](cluon::data::Envelope &&envelope)
          {
            auto msg = cluon::extractMessage<opendlv::proxy::stopDone>(std::move(envelope));
            countUp = msg.done();
          }

        };
        od4.dataTrigger(opendlv::proxy::stopDone::ID(), reachedStop);

            while (od4.isRunning()) {
                cv::Mat frame;

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
                    frame = wrapped.clone();
                }
                sharedMemory->unlock();
                String cascade_name = "car-28.xml";
           		if( !cascade.load( cascade_name ) )
           		{
              		  cout << "--(!)Error loading cascade\n";
             		   return -1;
            		};

		    if(countUp)
        	detectAndDisplay( frame );
            else{
            detectAndCountdown( frame );

            }
          }
            queue = carLeft + carForward + carRight;
                // Display image.
                if (VERBOSE) {
                    cv::imshow(sharedMemory->name().c_str(), frame);
                    std::cout << "carLeft: " << carLeft << std::endl;
                    std::cout << "carForward: " << carForward << std::endl;
                    std::cout << "carRight: " << carRight << std::endl;
                    std::cout << "queue: " << queue << std::endl;
                    cv::waitKey(1);

                }
            }
        }
        retCode = 0;
    }
    return retCode;
}
