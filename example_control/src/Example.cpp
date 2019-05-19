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

//nothing to see here, definitely not globals. please move along
    bool turn = false;
    bool follow = false;
    bool tempStop = false;
    std::string direction = "";
    std::string stopping = "";

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
        cluon::OD4Session od4Drive{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};
        cluon::OD4Session od4Turn{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};
        cluon::OD4Session od4Command{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};



        if (0 == od4Drive.isRunning() && 0 == od4Turn.isRunning())
        {
            std::cerr << "ERROR: No OD4Session running!!!" << std::endl;
            return -2;
        }

        const bool VERBOSE{commandlineArguments.count("verbose") != 0};
        //const float FREQ{(commandlineArguments["freq"].size() != 0) ? static_cast<float>(std::stof(commandlineArguments["freq"])) : static_cast<float>(-1.0)};


//-------_______-------___-------___---_________----____----_--_____________----_----___---_----_----________---_______-------___--------__---__----


        auto commandHandler{[&od4Command, &od4Drive, &od4Turn, VERBOSE, &turn, &follow, &direction](cluon::data::Envelope &&envelope)

            {

                //declear msg to sabe whats recieved from custom message
                auto msg = cluon::extractMessage<opendlv::proxy::instructions>(std::move(envelope));
                //trash shit to avoid errors

                const int16_t delay{500};

                opendlv::proxy::PedalPositionRequest pedalReq;
                opendlv::proxy::GroundSteeringRequest steerReq;

                //save content from message into tempSize
                turn = msg.turn(); // Corresponds to odvd message set
                follow = msg.follow();
                direction = msg.direction();

                if(VERBOSE){

                }

                    if(turn == true && direction == "left"){

                        steerReq.groundSteering(0.13);
                        od4Turn.send(steerReq);

                        //set speed
                        pedalReq.position(0.14);
                        od4Drive.send(pedalReq);

                        std::this_thread::sleep_for(std::chrono::milliseconds(6 * delay));

                        pedalReq.position(0.0);
                        od4Drive.send(pedalReq);

                        //reset wheels
                        steerReq.groundSteering(0.0);
                        od4Turn.send(steerReq);

//----___________--------__---_--______-----_--__---_-------___---_----_-------___---_______----_----_-------_-------_------

                    }else if(turn == true && direction == "right"){

                        steerReq.groundSteering(-0.37);
                        od4Turn.send(steerReq);

                        //set speed
                        pedalReq.position(0.14);
                        od4Drive.send(pedalReq);

                        std::this_thread::sleep_for(std::chrono::milliseconds(5 * delay));

                        pedalReq.position(0.0);
                        od4Drive.send(pedalReq);

                        //reset wheels
                        steerReq.groundSteering(0.0);
                        od4Turn.send(steerReq);

//------------_________-----_--___--__---______-----___---______-----_-__----_-------___-----___-----_----__----_----_--__---__------

                    }else if(turn == true && direction == "straight"){

                        steerReq.groundSteering(0.0);
                        od4Turn.send(steerReq);

                        pedalReq.position(0.15);
                        od4Drive.send(pedalReq);

                        std::this_thread::sleep_for(std::chrono::milliseconds(6 * delay));

                        pedalReq.position(0.0);
                        od4Drive.send(pedalReq);

                        steerReq.groundSteering(0.0);
                        od4Turn.send(steerReq);

//-------___________-----__------__________----_____---______-----_-----_____---_______----__---_-----_______----_______----______-----__---_--------                        

                    }else if(follow == true){

                    }

            }

        };

        od4Command.dataTrigger(opendlv::proxy::instructions::ID(), commandHandler);


//----------____________----__---__----________-----________--------------_-----__------____-------__---__------_---_---_--------______----------__


        float tempTurn{0.0};
        //declearing function and lambda function
        auto onTurn{[&od4Turn, VERBOSE, &tempTurn](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {

                std::cout << "recieved size from camera-microcontroler"  << std::endl;

                //declear msg to sabe whats recieved from custom message
                auto msg = cluon::extractMessage<opendlv::proxy::correctTurn>(std::move(envelope));
                //trash shit to avoid errors

                //save content from message into tempSize
                tempTurn = msg.offset(); // Corresponds to odvd message set

                opendlv::proxy::GroundSteeringRequest steerReq;

                if(VERBOSE){
                    
                }

                    if(tempTurn > -320 && tempTurn < -192){

                        steerReq.groundSteering(0.2);
                        od4Turn.send(steerReq);

                    }else if(tempTurn > -192 && tempTurn < -64){

                        steerReq.groundSteering(0.1);
                        od4Turn.send(steerReq);

                    }else if(tempTurn > 64 && tempTurn < 192){

                        steerReq.groundSteering(-0.1);
                        od4Turn.send(steerReq);

                    }else if(tempTurn > 192 && tempTurn < 320){

                        steerReq.groundSteering(-0.2);
                        od4Turn.send(steerReq);

                    }else{

                        steerReq.groundSteering(0.0);
                        od4Turn.send(steerReq);

                    }

                    std::cout <<"sending steering to control " << std::endl;

            }

        };


//--_________----_-------__-------_---_---_-------_----_--__---------------_----_-------__-------_----_----_--__----_----_----


        float tempDistReading{0.0};
        float frontDistance{0.0};
        auto onDistanceReading{[VERBOSE, &tempDistReading, &frontDistance](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {
                auto msg = cluon::extractMessage<opendlv::proxy::DistanceReading>(std::move(envelope));
                const uint16_t senderStamp = envelope.senderStamp(); // Local variables are not available outside the lambda function
                tempDistReading = msg.distance(); // Corresponds to odvd message set

                if(senderStamp == 0){

                    if (VERBOSE){
                        
                    }

                        frontDistance = tempDistReading;
                    
                }

            }
        };

        od4.dataTrigger(opendlv::proxy::DistanceReading::ID(), onDistanceReading);


//---------------_____-------------------------------__---------_____-----____-------------------________________---___-----_-------_-------_--


        //declearing function and lambda function
        auto onFollow{[&od4Drive, VERBOSE, &stopping, &frontDistance](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {

                // const int16_t delay{500};

                //std::cout << "recieved size from camera-microcontroler"  << std::endl;

                //declear msg to sabe whats recieved from custom message

                //trash shit to avoid errors
                auto msg2 = cluon::extractMessage<opendlv::proxy::stopRequest>(std::move(envelope));

                //save content from message into tempSize
                // Corresponds to odvd message set
                stopping = msg2.stopping();

                //declear message in order to send pedal request
                opendlv::proxy::PedalPositionRequest pedalReq;

                if(VERBOSE){
                    
                }

                    if(stopping == "stop"){

                        std::cout << "recieved: (" << stopping << ") prepaing to stop" << std::endl;

                        pedalReq.position(0.0);
                        od4Drive.send(pedalReq);

                    }else{

                    }

                    if(frontDistance > 0.2 && frontDistance < 1){

                        std::cout << "reacted on square, size is: (" << frontDistance << ") " << std::endl;

                        pedalReq.position(0.125);
                        od4Drive.send(pedalReq);

                    }else{

                        std::cout << "to close to car, stopping" << std::endl;

                        pedalReq.position(0.0);
                        od4Drive.send(pedalReq);

                    }
            }

        };

        od4Drive.dataTrigger(opendlv::proxy::correctTurn::ID(), onTurn);
        od4Drive.dataTrigger(opendlv::proxy::correctTurn::ID(), onFollow);


//-------_____________------__---__----_______----_____-----__---------_____---___---__________----__---___--__----________---__---__----_----_--__----_----_


        int counter = 0;
        float signSize{0.0};
        bool done = false;
        //declearing function and lambda function
        auto stopFollow{[&od4Turn, &od4Drive, &od4Command, VERBOSE, &frontDistance, &tempStop, &counter, &signSize, &done](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {

                std::cout <<"entering the stop function" << std::endl;

                //declear msg to sabe whats recieved from custom message
                auto msg1 = cluon::extractMessage<opendlv::proxy::stopReading>(std::move(envelope));

                tempStop = msg1.stop();

                signSize = msg1.signSize();

                //trash shit to avoid errors

                const int16_t delay{500};

                //std::this_thread::sleep_for(std::chrono::milliseconds(10* delay));

                opendlv::proxy::PedalPositionRequest pedalReq;
                opendlv::proxy::GroundSteeringRequest steerReq;
                opendlv::proxy::stopDone msg3;

                //declear message in order to send pedal request

                if(VERBOSE){
                    std::cout << "signsize outside while is: (" << signSize << ")" << std::endl;
                }

                    while(signSize < 4300 && frontDistance > 0.2 && frontDistance < 1.3){

                        std::cout << "signsize in while is: (" << signSize << ")" << std::endl;

                        pedalReq.position(0.125);
                        od4Drive.send(pedalReq);

                    }


                    msg3.done(done);
                    od4Command.send(msg3);


                    pedalReq.position(0.0);
                    od4Drive.send(pedalReq);

                    std::cout << " distance is: (" << frontDistance << ")" << std::endl;

                    if (frontDistance > 0.60 && counter < 1){

                        counter += 1;

                        steerReq.groundSteering(-0.02);
                        od4Turn.send(steerReq);

                        pedalReq.position(0.125);
                        od4Drive.send(pedalReq);

                        std::this_thread::sleep_for(std::chrono::milliseconds(5 * delay));

                        pedalReq.position(0.0);
                        od4Drive.send(pedalReq);

                        steerReq.groundSteering(0.0);
                        od4Turn.send(steerReq);

                        signSize = 0;

                        std::cout << "end of stop if, finished stopping" << std::endl;

                    }
            }

        };

        od4Command.dataTrigger(opendlv::proxy::stopReading::ID(), stopFollow);




//---------------______-----------____----_----__----____----_____----__----____________----__----_______----_----_______----_----------_--_______________---_-

        while(od4Drive.isRunning()){

            const int16_t delay{1000};

            std::cout << "od4 is running" << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(3 * delay));

        }

        return 0;
    }
}



