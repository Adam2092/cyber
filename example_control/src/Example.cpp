/*
 * Copyright (C) 2019 Yue Kang
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

#include <cstdint>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#include "cluon-complete.hpp"
#include "messages.hpp"

int32_t main(int32_t argc, char **argv) {


    std::cout << "this is testing the commandline";
    // Parse the arguments from the command line
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);



    if ( (0 == commandlineArguments.count("cid")) || (0 != commandlineArguments.count("help")) )
    {
        std::cerr << argv[0] << " is an example application for miniature vehicles (Kiwis) of DIT638 course." << std::endl;
        std::cerr << "Usage:  " << argv[0] << " --cid=<CID of your OD4Session> [--freq=<Frequency>] [--verbose] [--help]" << std::endl;
        std::cerr << "example:  " << argv[0] << " --cid=112 --freq=30 --verbose" << std::endl;
        return -1;
    }
    else
    {
        cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};

        if (0 == od4.isRunning())
        {
            std::cerr << "ERROR: No OD4Session running!!!" << std::endl;
            return -2;
        }

        const bool VERBOSE{commandlineArguments.count("verbose") != 0};
        //const float FREQ{(commandlineArguments["freq"].size() != 0) ? static_cast<float>(std::stof(commandlineArguments["freq"])) : static_cast<float>(-1.0)};

        // An example of message-sending function
        // Also an example of time-triggered function
        float tempDistReading{0.0};
        auto onDistanceReading{[&od4, VERBOSE, &tempDistReading](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {
                auto msg = cluon::extractMessage<opendlv::proxy::DistanceReading>(std::move(envelope));
                const uint16_t senderStamp = envelope.senderStamp(); // Local variables are not available outside the lambda function
                tempDistReading = msg.distance(); // Corresponds to odvd message set
                if (VERBOSE)
                {
                
                    std::cout << "Received DistanceReading message (senderStamp=" << senderStamp << "): " << tempDistReading << std::endl;
                
            }
            }
        };
        od4.dataTrigger(opendlv::proxy::DistanceReading::ID(), onDistanceReading);

//---------------_____-------------------------------__---------_____-----____-----------------

        float tempSize{0.0};

        auto onspeed{[&od4, VERBOSE, &tempSize](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {
            auto msg = cluon::extractMessage<opendlv::proxy::sizeReading>(std::move(envelope));
            const uint16_t senderStamp = envelope.senderStamp(); // Local variables are not available outside the lambda function
            tempSize = msg.size(); // Corresponds to odvd message set
            if(senderStamp == 0){
                                     std::cout << "testtestetsetestsetsetsetsetstet" << senderStamp << "): " << std::endl;
            }

            opendlv::proxy::PedalPositionRequest pedalReq;

            if(tempSize > 0 && tempSize < 10000000){

                pedalReq.position(0.1);
                od4.send(pedalReq);

                }
            }   

        };

        od4.dataTrigger(opendlv::proxy::sizeReading::ID(), onspeed);

/*
        float tempSteering{-0.35};

            	const uint16_t delay{1000};
                opendlv::proxy::GroundSteeringRequest steerReq;
                steerReq.groundSteering(tempSteering);
                od4.send(steerReq);

                    std::cout << "Sent GroundSteeringRequest message: " << tempSteering << std::endl;
                	opendlv::proxy::PedalPositionRequest pedalReq;
            		pedalReq.position(0.1);
            		od4.send(pedalReq);

            		std::this_thread::sleep_for(std::chrono::milliseconds(2 * delay));

					if (VERBOSE) std::cout << " and stop." << std::endl;

            		std::cout << "Sent GroundSteeringRequest message: " << tempSteering << std::endl;

            		pedalReq.position(0.0);
            		od4.send(pedalReq);

            		steerReq.groundSteering(0.0);
                	od4.send(steerReq);

                */



        return 0;
    }
}

