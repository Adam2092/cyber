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

int32_t turnLeft();
int32_t turnRight();
int32_t goStraight();

int32_t main(int32_t argc, char **argv) {


	bool turn = false;
	bool follow = false;
	//bool followCar = false;
	std::string direction = ""; 
	

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


//------------_______----___---__---_--__---_______----_______----_-------___--------____________-----_----___-----_






//_________---------______----------___-----_-------______---________-----_--_----___---___--__________----______--_-------__-_______----__------_-

      		  


//-------_______-------___-------___---_________----____----_--_____________----_----___---_----_----________---_______-------___--------__---__----

  auto commandHandler{[&od4Command, VERBOSE, &turn, &follow, &direction](cluon::data::Envelope &&envelope)

        	{

        		            std::cout << "recieved speed to car " << std::endl;


            
            //declear msg to sabe whats recieved from custom message
            auto msg = cluon::extractMessage<opendlv::proxy::instructions>(std::move(envelope));
            //trash shit to avoid errors

            //save content from message into tempSize
            turn = msg.turn(); // Corresponds to odvd message set
            follow = msg.follow();
            direction = msg.direction();



        		if(VERBOSE){

        			   if(turn == true && direction == "left"){
        			   	turnLeft();
        			   }else if(turn == true && direction == "right"){
        			   	turnRight();
        			   }else if(turn == true && direction == "straight"){
        			   	goStraight();
        			   }

        		}

        	}

        };

                	od4Command.dataTrigger(opendlv::proxy::instructions::ID(), commandHandler);      		  


//----------____________----__---__----________-----________--------------_-----__------____-------__---__------_---_---_--------______----------__
        
        float tempSpeed{0.0};
        auto adjustSpeed{[&od4Drive, VERBOSE, &tempSpeed](cluon::data::Envelope &&envelope)

        	{

        	std::cout << "recieved speed to car " << std::endl;

            auto msg = cluon::extractMessage<opendlv::proxy::sizeReading>(std::move(envelope));

        	const int16_t delay{1000};
        	
            opendlv::proxy::PedalPositionRequest pedalReq;

            tempSpeed = msg.speed(); // Corresponds to odvd message set



        		if(VERBOSE){

        		std::cout << "sending speed pedalReq " << std::endl;

        			std::this_thread::sleep_for(std::chrono::milliseconds(delay / 2));

        			pedalReq.position(tempSpeed);
                	od4Drive.send(pedalReq);

        		}

        	}

        };

        	    od4Drive.dataTrigger(opendlv::proxy::followSpeed::ID(), adjustSpeed);

       



//---------------_____-------------------------------__---------_____-----____-------------------________________---___-----_-------_-------_--
            
        //declearing variable to save message content
        float tempSize{0.0};
        float sizeCheck{0.0};
        float speed{0.05};
        float increase{0.03};
        float decrease{0.03};
        //declearing function and lambda function
        auto onFollow{[&od4Drive, VERBOSE, &tempSize, &speed](cluon::data::Envelope &&envelope)
            // &<variables> will be captured by reference (instead of value only)
            {
          	opendlv::proxy::followSpeed msg;

            	std::cout << "recieved size from camera-microcontroler"  << std::endl;

            //declear msg to sabe whats recieved from custom message
            auto msg1 = cluon::extractMessage<opendlv::proxy::sizeReading>(std::move(envelope));
            //trash shit to avoid errors

            //save content from message into tempSize
            sizeCheck = msg1.size(); // Corresponds to odvd message set

           
            //declear message in order to send pedal request
            opendlv::proxy::PedalPositionRequest pedalReq;
            if(VERBOSE){
            
            	tempSize = sizeCheck;

            	if(tempSize <= sizeCheck){

            		speed + increase;

            		tempSize = sizeCheck;

            	}else if(tempSize > sizeCheck){

            		speed - decrease;

            		tempSize = sizeCheck;

            	}else if(size > 55000)


            	

            	msg.speed(tempSpeed);

            	std::cout << "reacted on square, size is: (" << tempSize << ") " <<" sending: " << tempSpeed << " to adjust speed" << std::endl;

            	od4Drive.send(msg);


            
            std::cout << "sending speed to car " << std::endl;


            }

            }   

        };
        //on trigger, calls "onFollow" function as soon as message is revieced.
        while(followCar){

        	od4Drive.dataTrigger(opendlv::proxy::sizeReading::ID(), onFollow);

        }
        




//---------------______-----------____----_----__----____----_____----__----____________----__----_______----_----_______----_----------_--_______________---_-

        while(od4Drive.isRunning()){

        	const int16_t delay{1000};

        	std::cout << "od4 is running" << std::endl;

        	std::this_thread::sleep_for(std::chrono::milliseconds(3 * delay));

        }

        return 0;
    }
}




		int32_t turnLeft(){

        				   std::cout << "Turning left " << std::endl;


        					return 0;

        			}

      		  

//----------_____--------______---__---_____---___-----------_----__---___----------------_-----_-__----__---_____------_______----_______--------_-----

      	int32_t turnRight(){


        				   std::cout << "Turning right " << std::endl;


        				
        				   	return 0;
        			}

//----------_____--------______---__---_____---___-----------_----__---___----------------_-----_-__----__---_____------_______----_______--------_-----

      	int32_t goStraight(){


        				   std::cout << "going straight" << std::endl;


        				
        				   	return 0;
        			}
