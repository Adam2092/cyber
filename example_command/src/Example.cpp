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

        bool turnLeftNo = false;
        bool turnRightNo = false; 
        bool goStraightNo = false;
        bool carReady = false;
        
//-------_________-------__----_----_____---______--__----_----___-----__----__---_--------___-----__---------___----------_----_--__--

        auto commandHandler{[&od4, VERBOSE, &turnLeftNo, &turnRightNo, &goStraightNo](cluon::data::Envelope &&envelope)

            {

                std::cout << "recieved instructions from camera " << std::endl;


            
            //declear msg to sabe whats recieved from custom message
            auto msg = cluon::extractMessage<opendlv::proxy::signRec>(std::move(envelope));


            //save content from message into tempSize
            turnRightNo = msg.rightSign(); // Corresponds to odvd message set
            turnLeftNo = msg.leftSign();
            goStraightNo = msg.straightSign();


            }

        };

                    od4.dataTrigger(opendlv::proxy::signRec::ID(), commandHandler);  

//----_____________----__________---____--_----__---------_____----____________----__---______--______---__---_----_--_-----
        int counter = 0;


        auto ourTurn{[&od4, VERBOSE, &carReady, &counter](cluon::data::Envelope &&envelope)

            {

                if (counter == 0){

                std::cout << "it is now our turn to go " << std::endl;
                counter += 1;
                
                }

            
            //declear msg to sabe whats recieved from custom message
            auto msg = cluon::extractMessage<opendlv::proxy::goTime>(std::move(envelope));


            //save content from message into tempSize
            carReady = msg.ready(); // Corresponds to odvd message set
          


            }

        };

                    od4.dataTrigger(opendlv::proxy::goTime::ID(), ourTurn);  



//-------_________----------_____---__----_----____----______--____________-----_-___---_----________---------_____---___--__----_-


        int input;
        std::cout << "" << std::endl;
        std::cout << "enter 1 to turn left" << std::endl;
        std::cout << "enter 2 to turn right" << std::endl;
        std::cout << "enter 3 to go straight" << std::endl;
        std::cout << "enter 4 to follow" << std::endl;

        opendlv::proxy::instructions msg;

        if(VERBOSE){

        std::cin >> input;

        if(input == 0){
            std::cout << "input was 0" << std::endl;
        }

        }
       
        bool turn = true;
        bool follow = true;


        while(od4.isRunning()){

        	const int16_t delay{1000};

            std::cin >> input;

            if(carReady == false){

                std::cout << "wait for your turn" << std::endl;

            }else if(input == 1 && turnLeftNo == false){
                msg.turn(turn);

                msg.direction("left");

                od4.send(msg);

                std::cout << "sending turn left" << std::endl;


            }else if(input == 1 && turnLeftNo == true){

                std::cout << "You are not allowed to turn left here" << std::endl;

            }else if(input == 2 && turnRightNo == false){
                msg.turn(turn);

                msg.direction("right");

                od4.send(msg);

                std::cout << "sending turn right" << std::endl;


            }else if(input == 2 && turnRightNo == true){

                std::cout << "you are not allowed to turn right here" << std::endl;

            }else if(input == 3 && goStraightNo == false){
                msg.turn(turn);

                msg.direction("straight");

                od4.send(msg);

                std::cout << "sending follow" << std::endl;

            }else if(input == 3 && goStraightNo == true){

                std::cout << "you are not allowed to go straight" << std::endl;

            }else if(input == 4){

                msg.follow(follow);

                od4.send(msg);

                std::cout << "sending follow" << std::endl;

            }else{

                std::cout << "enter something real stupid" << std::endl;

            }

            input = 0;

        	//std::cout << "od4 is running" << std::endl;

        	std::this_thread::sleep_for(std::chrono::milliseconds(3 * delay));

        }

        return 0;
    }
}

