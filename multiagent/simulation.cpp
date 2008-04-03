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

#define UPDATE_INTERVAL 0.04 //every this amount of seconds, posion and speed vectors will be recalculated
#define ROUND_LENGTH 12.0 //round length in seconds
#include "robot.cpp"
#include "ann.cpp"

class Simulation
{
  private:
    PoolMember member_;
    Environment* environment_;
  public:
    Simulation(PoolMember &member, Environment* environment)
    {
      member_ = member;
      environment_ = environment;
    }
    void runSim(bool);
    double getScore()
    {
      return member_.fitness;
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

void Simulation::runSim(bool printprogress)
{
  if (printprogress)
    prepareSimulationFile();
  //robot initialization
  Robot robots[ROBOT_COUNT];
  for (int i = 0; i < ROBOT_COUNT; ++i)
  {
    robots[i].initialize(environment_);
  }
  double robotStartPositions[ROBOT_COUNT][2] = { { 0.15, -0.1 },
      { 0.15, -0.25 },
      { 0.15, -0.40 },
      { 0.30, -0.15 },
      { 0.30, -0.30 } };
  double robotEndPositions[ROBOT_COUNT][2] = { { 0.60, -0.30 },
      { 0.60, -0.15 },
      { 0.75, -0.40 },
      { 0.75, -0.25 },
      { 0.75, -0.10 } };
  for (int i = 0; i < ROBOT_COUNT; ++i)
  {
    robots[i].position_[0] = robotStartPositions[i][0];
    robots[i].position_[1] = robotStartPositions[i][1];
  }
  Ann anns[ROBOT_COUNT];
  for (int i = 0; i < ROBOT_COUNT; ++i)
  {
    double* start = member_.gene + (WEIGHT_COUNT + SENSOR_COUNT) * i;
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
  do
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
        int index = (WEIGHT_COUNT + SENSOR_COUNT) * (nr + 1) - SENSOR_COUNT;
        //their states are transmited to ANN inputs
        anns[nr].setNode(i + 1, robots[nr].sensorDistToWall(/*55.0)); //*/member_.gene[index + i]));
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

      anns[nr].setNode(SENSOR_COUNT + 3, robots[nr].angleToPoint(
          robotEndPositions[nr][0], robotEndPositions[nr][1], 0));
      double distanceToDestinationSq = robots[nr].distToPointSq(
          robotEndPositions[nr][0], robotEndPositions[nr][1]);
      anns[nr].setNode(SENSOR_COUNT + 4, distanceToDestinationSq);

      anns[nr].process();
      //      printf("%f %f\n", ann.getOutputNode(0), (ann.getOutputNode(1) - 0.5) * 2);

      // New position is counted. ANN's outputs are acceleration and rotation
      // speed. Values from ANN are from 0 to 1 so they must be converted to 
      // [-1;1] for normal operation.
      robots[nr].newPosition(anns[nr].getOutputNode(2),
          (anns[nr].getOutputNode(1) - 0.5) * 2.0);
      /*ann.getNode(ann.getNodeCount()-2),
       (ann.getNode(ann.getNodeCount()-1)-0.5)*2.0);*/

      if (printprogress)
      {
        double data[] = { robots[nr].position_[0], robots[nr].position_[1],
            robots[nr].head_ };
        outputSimulationStep(data, sizeof(data) / sizeof(double));
      }
      simulationFitness -= distanceToDestinationSq;
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
  } while (step * UPDATE_INTERVAL < ROUND_LENGTH && !stop);
  member_.fitness = simulationFitness;
}
