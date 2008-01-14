//     This file is part of ARTIcomm.
//
//     ARTIcomm is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     ARTIcomm is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with ARTIcomm.  If not, see <http://www.gnu.org/licenses/>.


#include <math.h>
using namespace std;

#define IN1 (4+1)  //number of input nodes+1
#define HID1 (2+1)  //number of hidden nodes+1
#define OUT1 0  //output nodes
#define wcount1 (IN1*(HID1-1)+HID1*OUT1) //number of weights

#define IN2 (6+1)  //number of input nodes+1
#define HID2 (3+1)  //number of hidden nodes+1
#define OUT2 2  //output nodes
#define wcount2 (IN2*(HID2-1)+HID2*OUT2) //number of weights

#define dydis 120  // size of the population
#define amplitude 1500  //how many times variable can be increased/decreased in mutation
#define GEN (dydis*30)  //how many evaluations must pass for status to be outputed

#define maxs 0.15 //max speed/s
#define maxrot 120 //max rotation degrees/s
#define minrot 60  //min ratation degrees/s
#define maxacc 0.15  //max acceleration m/s/s
#define t 0.038 //every this amount of seconds, posion and speed vectors will be recalculated
#define roundl 35 //round length in seconds
#define spindulys 0.05 //robots radius in metres
#define maxalloweddist 0.3 //if below this distance from the wall, penality will be applied
#define maxsensdist 2  //how many ship sized distance units can sensor see in front
#define sensorcount 2 //how many sensors
#define genecount (sensorcount + wcount1 + wcount2)
#define sectostarve 17
#define sectosleep 17

#define deginrad (3.14159265/180)

#define startx 0.45
#define starty -0.3
#define startrot 180

//structure for one member. gene is an array for weights and fittness is member's gene
struct TPopul {
  double gene[genecount];
  double fitness;
};
