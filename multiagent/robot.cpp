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

#define MAX_SPEED 0.15
#define MAX_ROTATION_SPEED 120.0 //max rotation degrees/s
#define MIN_ROTATION_SPEED 60.0  //min ratation degrees/s
#define MAX_ACCELERATION 0.2  //max acceleration m/s/s
#define ROBOTS_RADIUS 0.05
#define ROBOTS_RADIUSSQ 0.0025 //robots radius in metres 0.05
#define MAX_DISTANCE_SEEN 2.0
#define MAX_DISTANCE_SEENSQ 4.0  //how many ship sized distance units can sensor see in front 2.0
#define START_ROTATION 180.0

#define RAD_IN_DEG (3.14159265/180.0)

#include "environment.cpp"

class Robot
{
  public:
    void initialise(Environment*);
    void rotate(double rot); //rotates speed vector. needs desired rotation speed and current speed
    void accelerate(double acc); //accelerates speed vector
    void newPosition(double, double); //calculates new osition. needs acceleration (0 to 1) and rotaion speed (0 to 1)
    double sensorDistToWall(double); //returns sensor value (0 to 1). 1 is equals maxsensdist or more
    double distToWallSq(); //returns distance in metres to nearest wall
    double distToPointSq(double &x, double &y);
    double angleToPoint(double &x, double &y, double dhead); //returns angle to a point when dhead is angle from heading

    double speed_;
    double head_; // heading starting from east and increasing counter-clockwise
    double position_[2]; //position
  private:
    Environment* environment_;
};

#define PI 3.1415926535897932384626433832795
#define PI05 1.57079632679489661923132169163975
#define PI2 6.283185307179586476925286766559
#define PI15 4.71238898038468985769396507491925

INL double fastSinInRange(const double& fAngle)
{
  double x2 = fAngle * fAngle;
  double x3 = x2 * fAngle;
  double x5 = x2 * x3;
  return 7.61e-03 * x5 - 1.6605e-01 * x3 + fAngle;
}

INL double fastSin(double x)
{
  while (x < -PI05)
    x += PI2;
  while (x > PI15)
    x -= PI2;
  if (x < PI05)
    return fastSinInRange(x);
  else
    return fastSinInRange(PI - x);
}

INL double fastCos(const double& x)
{
  return fastSin(x + PI05);
}

INL void Robot::initialise(Environment* environment)
{
  speed_ = 0.0;
  head_ = START_ROTATION;
  environment_ = environment;
}

INL double Robot::sensorDistToWall(double dhead)
{
  double a1[2], b1[2], b2[2], B[2], v1[2], v2[2], det11, det12, det2, t1, t2;
  //absolute heading of a sensor
  double newhead = head_ + dhead;
  //a2 and a1 is calculated. Then we have v1 which is vector of a sensor. Its length is spindulys.
  a1[0] = position_[0];
  a1[1] = position_[1];
  v1[0] = fastCos(newhead * RAD_IN_DEG) * ROBOTS_RADIUS;
  v1[1] = fastSin(newhead * RAD_IN_DEG) * ROBOTS_RADIUS;
  //prepearing to calculate shortest distance to wall
  double minDistance = 1.0, distance;
  for (int i = 0; i < environment_->wallCount(); i++)
  {
    //wall's b1, b2 and v2, where v2 is walls length
    b1[0] = environment_->getCorner(i, 0);
    b1[1] = environment_->getCorner(i, 1);
    b2[0] = environment_->getCorner(i, 2);
    b2[1] = environment_->getCorner(i, 3);
    v2[0] = b2[0] - b1[0];
    v2[1] = b2[1] - b1[1];
    //B is calculated, which is the distance vector from a1 to b1
    B[0] = b1[0] - a1[0];
    B[1] = b1[1] - a1[1];
    //determinants are counted. I don't know how exactly everything is tied up, but it works
    det11 = B[0] * v2[1] - B[1] * v2[0];
    det12 = v1[0] * B[1] - v1[1] * B[0];
    det2 = v1[0] * v2[1] - v1[1] * v2[0];
    //checks whether sensor and wall aren't parallel
    if (det2 != 0.0)
    {
      //t1 is distance to wall in units of spindulys + 1
      t1 = det11 / det2;
      //t2 is a place in wall vector, where sensor is pointing
      t2 = -det12 / det2;
      //check if censor is "hitting" the wall and if it is not behind the sensor
      if (t2 <= 1.0 && t2 >= 0.0 && t1 >= 0.0)
      {
        //distance now is distance to wall in units of spindulys
        distance = t1 - 1.0;
        //if distance is more than sensor can see
        if (distance >= MAX_DISTANCE_SEEN)
          distance = 1.0;
        else
        {
          //if robot is hitting a wall
          if (distance < 0.0)
            distance = 0.0;
          else
          {
            //distance is finally double between 0 and 1 if it is in range
            distance /= MAX_DISTANCE_SEEN;
          }
        }
      }
      else
        distance = 1.0;
    }
    //if wall and sensor are parallel
    else
      distance = 1.0;
    //check if this is the smallest distance
    if (distance < minDistance)
      minDistance = distance;
  }
  //returns global distance as double between 0 and 1
  return minDistance;
}

//algorithm is borrowed from some delphi site
INL double Robot::distToWallSq()
{
  double p1, p2, x1, y1, x2, y2, dx, dy, Ratio, l1, l2;
  //simmilar to CRobot::sensordisttowall. gdistance is made to be as large as possible, so that first
  //  calculation would overwrite it
  double minDistanceSq = 5000000.0, distanceSq;
  for (int i = 0; i < environment_->wallCount(); i++)
  {
    p1=position_[0];
    p2=position_[1];
    x1=environment_->getCorner(i, 0);
    y1=environment_->getCorner(i, 1);
    x2=environment_->getCorner(i, 2);
    y2=environment_->getCorner(i, 3);
    dx = x2 - x1;
    dy = y2 - y1;
    //ratio is the place, where distance from robot is hitting the segment
    Ratio = ((p1 - x1) * dx + (p2 - y1) * dy) / (dx * dx + dy * dy);
    //if distance is hitting not in segment, distance is calculated by simple pitagorian rules
    if (Ratio < 0)
      distanceSq = (p1 - x1) * (p1 - x1) + (p2 - y1) * (p2 - y1);
    else if (Ratio > 1)
      distanceSq = (p1 - x2) * (p1 - x2) + (p2 - y2) * (p2 - y2);
    else
    //if distance is hitting in the segment-wall, then distance is calculated differently
    {
      l1 = (1 - Ratio) * x1 + Ratio * x2;
      l2 = (1 - Ratio) * y1 + Ratio * y2;
      distanceSq = (p1 - l1) * (p1 -l1) + (p2 - l2) * (p2 - l2);
    }
    if (distanceSq < minDistanceSq)
      minDistanceSq = distanceSq;
  }
  return minDistanceSq - ROBOTS_RADIUSSQ;
}

INL double Robot::distToPointSq(double &x, double &y)
{
  double dx = position_[0] - x;
  double dy = position_[1] - y;
  return (dx)*(dx)+(dy)*(dy);
}

INL double Robot::angleToPoint(double &x, double &y, double deltaHeading)
{
  double angle = atan2(y - position_[1], x - position_[0]) / RAD_IN_DEG;
  angle -= head_ + deltaHeading;
  return angle;
}

INL void Robot::rotate(double rot)
{
  //maximum allowed turning rate at current speed
  double alfa= MAX_ROTATION_SPEED - (MAX_ROTATION_SPEED - MIN_ROTATION_SPEED)
  * speed_ / MAX_SPEED;
  //current rotation speed in rads/s
  alfa *= rot * UPDATE_INTERVAL * RAD_IN_DEG;
  //heading is updated and forced to appear in [-180;180] bounds
  head_ += alfa / RAD_IN_DEG;
  if (head_ < -180.0)
    head_ += 360;
  else if (head_ > 180.0)
    head_ -= 360;
}

INL void Robot::accelerate(double acc)
{
  if (speed_ > MAX_SPEED * acc)
    speed_ -= MAX_ACCELERATION * UPDATE_INTERVAL;
  else
    speed_ += MAX_ACCELERATION * UPDATE_INTERVAL;
  if (speed_ < 0.0)
    speed_ = 0.0;
}

INL void Robot::newPosition(double acc, double rot)
{
  //let's rotate speed vector
  rotate(rot);
  //let's accelerate speed vector
  accelerate(acc);
  //new position is calculated
  double speedX = fastCos(head_ * RAD_IN_DEG) * speed_;
  double speedY = fastSin(head_ * RAD_IN_DEG) * speed_;
  position_[0] += speedX * UPDATE_INTERVAL;
  position_[1] += speedY * UPDATE_INTERVAL;
}
