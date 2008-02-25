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
    ~Simulation()
    {
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

#define ROBOT_COUNT 3

  Robot robots[ROBOT_COUNT];
  for (int i = 0; i < ROBOT_COUNT; ++i)
  {
    robots[i].initialize(environment_);
  }
  for (int i = 0; i < ROBOT_COUNT; ++i)
  {
    robots[i].position_[0] = 0.15;
    robots[i].position_[1] = i * (-0.2) - 0.1;
  }
  Ann ann(member_.gene, member_.gene + WEIGHT_COUNT, IN, HID, OUT);

  //fittnes is starting from zero
  float simulationFitness = 0;
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
    step++;
    for (int nr = 0; nr < ROBOT_COUNT; ++nr)
    {
      // checking for wall collision
      if (robots[nr].distToWall() < 0)
      {
        robots[nr].position_[0] = oldPosition[nr][0];
        robots[nr].position_[1] = oldPosition[nr][1];
        robots[nr].speedVector_[0] = 0.0;
        robots[nr].speedVector_[1] = 0.0;
      }

      // checking for robot collision
      double shortestDistance = 100000000.0;
      double angleToRobot = 180.0;
      double shortestGlobalDistance = 100000000.0;
      for (int i = 0; i < ROBOT_COUNT; ++i)
      {
        if (i != nr)
        {
          double currentDistance = robots[nr].distToPoint(
              robots[i].position_[0], robots[i].position_[1]);
          double currentAngle = robots[nr].angleToPoint(robots[i].position_[0],
              robots[i].position_[1], 0.0);
          if (fabs(currentAngle) < 90.0 && shortestDistance > currentDistance)
          {
            shortestDistance = currentDistance;
            angleToRobot = currentAngle;
          }
          if (shortestGlobalDistance > currentDistance)
            shortestGlobalDistance = currentDistance;
        }
      }
      if (shortestGlobalDistance < ROBOTS_RADIUS * 2)
      {
        robots[nr].position_[0] = oldPosition[nr][0];
        robots[nr].position_[1] = oldPosition[nr][1];
        robots[nr].speedVector_[0] = 0.0;
        robots[nr].speedVector_[1] = 0.0;
      }

      oldPosition[nr][0] = robots[nr].position_[0];
      oldPosition[nr][1] = robots[nr].position_[1];

      //all sensor states are being updated
      ann.clearNodes();
      for (int i = 0; i < SENSOR_COUNT; i++)
        //their states are transmited to ANN inputs
        ann.setNode(i + 1,
            robots[nr].sensorDistToWall(member_.gene[WEIGHT_COUNT+i]));

      if (fabs(angleToRobot) > 90.0 || shortestDistance > MAX_DISTANCE_SEEN
          * ROBOTS_RADIUS)
      {
        ann.setNode(SENSOR_COUNT + 1, 0.0); // null angle
        ann.setNode(SENSOR_COUNT + 2, 1.0); // max distance 1.0
      }
      else
      {
        ann.setNode(SENSOR_COUNT + 1, angleToRobot);
        ann.setNode(SENSOR_COUNT + 2, shortestDistance / ROBOTS_RADIUS - 1); // [0, 1]
      }

      ann.setNode(SENSOR_COUNT + 3, robots[nr].angleToPoint(0.75, (2 - nr)
          * (-0.2) - 0.1, 0.0));
      double distanceToDestination = robots[nr].distToPoint(0.75, (2 - nr)
          * (-0.2) - 0.1);
      ann.setNode(SENSOR_COUNT + 4, distanceToDestination);

      ann.process();
//      printf("%f %f\n", ann.getOutputNode(0), (ann.getOutputNode(1) - 0.5) * 2);

      // New position is counted. ANN's outputs are acceleration and rotation
      // speed. Values from ANN are from 0 to 1 so they must be converted to 
      // [-1;1] for normal operation.
      robots[nr].newPosition(/*member_.gene[0],*/ann.getOutputNode(-1), (ann.getOutputNode(0) - 0.5) * 2);
      if (printprogress)
      {
        double data[] = { robots[nr].position_[0], robots[nr].position_[1],
            robots[nr].head_};
        outputSimulationStep(data, sizeof(data) / sizeof(double));
      }
      simulationFitness -= distanceToDestination;
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
