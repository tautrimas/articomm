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

#define IN 6   //number of input nodes
#define HID 4  //number of hidden nodes
#define OUT 2  //output nodes
#define WEIGHT_COUNT ((IN+1)*(HID)+(HID+1)*OUT) //number of weights

#define POPULATION_SIZE 12  // size of the population
#define EVALUATIONS_LIMIT (POPULATION_SIZE*15)  //how many evaluations must pass for status to be outputed

#define MAX_SPEED 0.15 //max speed/s
#define MAX_ROTATION_SPEEED 120.0 //max rotation degrees/s
#define MIN_ROTATION_SPEED 60.0  //min ratation degrees/s
#define MAX_ACCELERATION 0.15  //max acceleration m/s/s
#define UPDATE_INTERVAL 0.04 //every this amount of seconds, posion and speed vectors will be recalculated
#define ROUND_LENGTH 12.0 //round length in seconds
#define ROBOTS_RADIUS 0.05 //robots radius in metres
#define MAX_DISTANCE_SEEN 2.0  //how many ship sized distance units can sensor see in front
#define SENSOR_COUNT 2 //how many sensors
#define GENE_COUNT (SENSOR_COUNT + WEIGHT_COUNT)

#define RAD_IN_DEG (3.14159265/180.0)

#define START_ROTATION 180.0

#define INL

//structure for one member. gene is an array for weights and fittness is member's gene
struct PoolMember {
  double gene[GENE_COUNT];
  double fitness;
};
