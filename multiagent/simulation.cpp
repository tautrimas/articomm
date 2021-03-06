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

#include "robot.cpp"
#include "ann.cpp"

#define IN 6   //number of input nodes
#define HID 2  //number of hidden nodes
#define OUT 0  //output nodes
#define WEIGHT_COUNT ((IN+1)*(HID)+(HID+1)*OUT) //number of weights
#define SENSOR_COUNT 3 //how many sensors
#define ROBOT_COUNT 3
#define GENE_COUNT ((SENSOR_COUNT + WEIGHT_COUNT) * ROBOT_COUNT)
#define ROUND_LENGTH 24.0 //round length in seconds
struct PoolMember
{
    double gene[GENE_COUNT];
    double fitness;
};

class Simulation
{
  private:
    PoolMember* member_;
    Environment* environment_;
  public:
    Simulation(PoolMember* member, Environment* environment)
    {
      member_ = member;
      environment_ = environment;
    }
    void runSim(bool);
    double getScore()
    {
      return member_->fitness;
    }
    void prepareSimulationFile();
    void outputSimulationStep(double*, int);
};

void Simulation::prepareSimulationFile()
{
  FILE* wout;
  wout = fopen("coords.txt", "w");
  fclose(wout);
}

void Simulation::outputSimulationStep(double* data, int dataSize)
{
  FILE* wout;
  wout = fopen("coords.txt", "a");
  fprintf(wout, "\n");
  for (int i = 0; i < dataSize; ++i)
  {
    fprintf(wout, " %0.4f", data[i]);
    //printf("%0.4f ", data[i]);
  }
  //printf("\n");
  fclose(wout);
}

void Simulation::runSim(bool printProgress)
{
  if (printProgress)
  {
    prepareSimulationFile();
    double data[] = { ROBOT_COUNT + 0.1, FIRE_COUNT + 0.1 };
    outputSimulationStep(data, sizeof(data) / sizeof(double));
  }
  //robot initialization
  Robot robots[ROBOT_COUNT];
  for (int i = 0; i < ROBOT_COUNT; ++i)
  {
    robots[i].initialise(environment_);
  }
  double robotStartPositions[ROBOT_COUNT][2] = { { 0.3, 0.6 }, { 0.2, 0.5 }, {
      0.1, 0.6 } };
  for (int i = 0; i < ROBOT_COUNT; ++i)
  {
    robots[i].position_[0] = robotStartPositions[i][0];
    robots[i].position_[1] = robotStartPositions[i][1];
  }
  Ann anns[ROBOT_COUNT];
  for (int i = 0; i < ROBOT_COUNT; ++i)
  {
    double* start = member_->gene + (WEIGHT_COUNT + SENSOR_COUNT) * i;
    double* end = start + WEIGHT_COUNT;
    anns[i].initialise(start, end, IN, HID, OUT);
  }

  //fittnes is starting from zero
  double simulationFitness = 0;
  //step*t is the second being virtualized
  int step = 0;
  bool stop = false;
  double oldPosition[ROBOT_COUNT][2];
  /*  bool didReachDestination[ROBOT_COUNT];
   for (int i = 0; i < ROBOT_COUNT; ++i)
   {
   didReachDestination[i] = false;
   }*/
  double extinguished[FIRE_COUNT] = { 0 };

  if (printProgress)
  {
    for (int i = 0; i < FIRE_COUNT; ++i)
    {
      double data[] = { environment_->getFire(i, 0),
          environment_->getFire(i, 1) };
      outputSimulationStep(data, sizeof(data) / sizeof(double));
    }
  }
  while (step * UPDATE_INTERVAL < ROUND_LENGTH && !stop)
  {
    ++step;

    double distanceMatrix[ROBOT_COUNT][ROBOT_COUNT];
    double angleMatrix[ROBOT_COUNT][ROBOT_COUNT];
    bool didMove[ROBOT_COUNT];
    for (int i = 0; i < ROBOT_COUNT; ++i)
    {
      for (int j = 0; j < ROBOT_COUNT; ++j)
      {
        distanceMatrix[i][j] = -1.0;
      }
      didMove[i] = false;
    }

    for (int nr = 0; nr < ROBOT_COUNT; ++nr)
    {
      // checking for wall collision
      if (robots[nr].distToWallSq() < 0)
      {
        robots[nr].position_[0] = oldPosition[nr][0];
        robots[nr].position_[1] = oldPosition[nr][1];
        robots[nr].speed_ = 0.0;
        didMove[nr] = true;
        // stop = true;
        simulationFitness += -1.0;
      }

      // checking for robot collision
      double shortestDistanceSq = 100000000.0;
      double angleToRobot = 180.0;
      double shortestGlobalDistanceSq = 100000000.0;

      for (int i = 0; i < ROBOT_COUNT; ++i)
      {
        if (i != nr)
        {
          double currentDistanceSq;
          double currentAngle;

          if (!didMove[i] && distanceMatrix[nr][i] > -0.9)
          {
            currentDistanceSq = distanceMatrix[nr][i];
            currentAngle = angleMatrix[nr][i];
          }
          else
          {
            currentDistanceSq = robots[nr].distToPointSq(
                robots[i].position_[0], robots[i].position_[1]);
            currentAngle = robots[nr].angleToPoint(robots[i].position_[0],
                robots[i].position_[1], 0.0);

            angleMatrix[nr][i] = currentAngle;
            angleMatrix[i][nr] = -robots[i].head_ - 180 + currentAngle
                + robots[nr].head_;
            if (angleMatrix[i][nr] < -180.0)
              angleMatrix[i][nr] += 360;
            else if (angleMatrix[i][nr] > 180.0)
              angleMatrix[i][nr] -= 360;

            if (angleMatrix[i][nr] < -180.0)
              angleMatrix[i][nr] += 360;
            else if (angleMatrix[i][nr] > 180.0)
              angleMatrix[i][nr] -= 360;

            distanceMatrix[nr][i] = currentDistanceSq;
            distanceMatrix[i][nr] = currentDistanceSq;

          }
          if (fabs(currentAngle) < 90.0 && shortestDistanceSq
              > currentDistanceSq)
          {
            shortestDistanceSq = currentDistanceSq;
            angleToRobot = currentAngle;
          }
          if (shortestGlobalDistanceSq > currentDistanceSq)
            shortestGlobalDistanceSq = currentDistanceSq;
        }
      }
      if (shortestDistanceSq < ROBOTS_RADIUSSQ * 4)
      {
        robots[nr].position_[0] = oldPosition[nr][0];
        robots[nr].position_[1] = oldPosition[nr][1];
        robots[nr].speed_ *= 0.8;
        didMove[nr] = true;
      }

      oldPosition[nr][0] = robots[nr].position_[0];
      oldPosition[nr][1] = robots[nr].position_[1];

      //all sensor states are being updated
      anns[nr].clearNodes();
      for (int i = 0; i < SENSOR_COUNT; i++)
      {
        //int index = (WEIGHT_COUNT + SENSOR_COUNT) * (nr + 1) - SENSOR_COUNT;
        //their states are transmited to ANN inputs
        anns[nr].setNode(i + 1, robots[nr].sensorDistToWall(60.0 * i - 60.0)/*member_->gene[index + i]) * 90.0*/);
      }

      if (fabs(angleToRobot) > 90.0 || shortestDistanceSq > MAX_DISTANCE_SEENSQ
      * ROBOTS_RADIUSSQ)
      {
        anns[nr].setNode(SENSOR_COUNT + 1, 0.0); // null angle
        anns[nr].setNode(SENSOR_COUNT + 2, 1.0); // max distance 1.0
      }
      else
      {
        anns[nr].setNode(SENSOR_COUNT + 1, angleToRobot);
        anns[nr].setNode(SENSOR_COUNT + 2, shortestDistanceSq / ROBOTS_RADIUSSQ
        - 1); // incorrect!!
      }

      /*anns[nr].setNode(SENSOR_COUNT + 1, 0.0); // null angle
       anns[nr].setNode(SENSOR_COUNT + 2, 1.0); // max distance 1.0*/

      double heatMeter = 0.0;
      for (int i = 0; i < FIRE_COUNT; ++i)
      {
        if (extinguished[i] < 5.0)
        {
          double xSq = robots[nr].distToPointSq(environment_->getFire(i, 0),
              environment_->getFire(i, 1));
          heatMeter += exp(-9.20997 * xSq);
          if (xSq < 0.0225)
          {
            extinguished[i] += UPDATE_INTERVAL;
          }
        }
        else
        {
          simulationFitness += 1.1 * UPDATE_INTERVAL;
        }
      }

      anns[nr].setNode(SENSOR_COUNT + 3, heatMeter);
      /*
       anns[nr].setNode(SENSOR_COUNT + 3, robots[nr].angleToPoint(
       robotEndPositions[nr][0], robotEndPositions[nr][1], 0));
       
       double distanceToDestinationSq = robots[nr].distToPointSq(
       robotEndPositions[nr][0], robotEndPositions[nr][1]);
       anns[nr].setNode(SENSOR_COUNT + 4, distanceToDestinationSq);*/

      anns[nr].process();

      // New position is counted. ANN's outputs are acceleration and rotation
      // speed. Values from ANN are from 0 to 1 so they must be converted to 
      // [-1;1] for normal operation.
      robots[nr].newPosition(anns[nr].getOutputNode(2),
          (anns[nr].getOutputNode(1) - 0.5) * 2.0);
      /*ann.getNode(ann.getNodeCount()-2),
       (ann.getNode(ann.getNodeCount()-1)-0.5)*2.0);*/

      if (printProgress)
      {
        double data[] = { robots[nr].position_[0], robots[nr].position_[1],
            robots[nr].head_ };
        outputSimulationStep(data, sizeof(data) / sizeof(double));
      }
      simulationFitness += heatMeter * UPDATE_INTERVAL;
      /*if (distanceToDestination < ROBOTS_RADIUS)
       didReachDestination[nr] = true;
       bool allReachedDestinations = true;
       for (int i = 0; i < ROBOT_COUNT; ++i)
       {
       if (didReachDestination[i] == false)
       allReachedDestinations = false;
       }
       if (allReachedDestinations == true)
       stop = true;*/
    }
    if (printProgress)
    {
      for (int i = 0; i < FIRE_COUNT; ++i)
      {
        double data[] = { 0.1 - extinguished[i] * 0.02 };
        outputSimulationStep(data, sizeof(data) / sizeof(double));
      }
    }
  }
  member_->fitness = simulationFitness;
}
