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

#include "environment.cpp"

class Robot
{
  public:
    Robot()
    {
    }
    ;
    ~Robot()
    {
    }
    void initialize(Environment*);
    void rotate(double vect[], double rot, double s); //rotates speed vector. needs desired rotation speed and current speed
    void newPosition(double, double); //calculates new osition. needs acceleration (0 to 1) and rotaion speed (0 to 1)
    double sensorDistToWall(double); //returns sensor value (0 to 1). 1 is equals maxsensdist or more
    double distToWall(); //returns distance in metres to nearest wall
    double distToPoint(double &x, double &y);
    double angleToPoint(double &x, double &y, double dhead); //returns angle to a point when dhead is angle from heading

    double speedVector_[2]; //speedvector
    double head_; // heading starting from east and increasing counter-clockwise
    double position_[2]; //position
  private:
    Environment* environment_;
};

INL void Robot::initialize(Environment* environment)
{
  speedVector_[0] = 0;
  speedVector_[1] = 0;
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
  v1[0] = cos(newhead*RAD_IN_DEG)*ROBOTS_RADIUS;
  v1[1] = sin(newhead*RAD_IN_DEG)*ROBOTS_RADIUS;
  //prepearing to calculate shortest distance to wall
  double gdistance = 1, distance;
  for (int i = 0; i < environment_ -> wallcount(); i++)
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
    if (det2 != 0)
    {
      //t1 is distance to wall in units of spindulys + 1
      t1 = det11 / det2;
      //t2 is a place in wall vector, where sensor is pointing
      t2 = -det12 / det2;
      //check if censor is "hitting" the wall and if it is not behind the sensor
      if (t2 <= 1 && t2 >= 0 && t1 >= 0)
      {
        //distance now is distance to wall in units of spindulys
        distance = t1 - 1;
        //if distance is more than sensor can see
        if (distance >= MAX_DISTANCE_SEEN)
          distance = 1;
        else
        {
          //if robot is hitting a wall
          if (distance < 0)
            distance = 0;
          else
          {
            //distance is finally double between 0 and 1 if it is in range
            distance /= MAX_DISTANCE_SEEN;
          }
        }
      }
      else
        distance = 1;
    }
    //if wall and sensor are parallel
    else
      distance = 1;
    //check if this is the smallest distance
    if (distance < gdistance)
      gdistance = distance;
  }
  //returns global distance as double between 0 and 1
  return gdistance;
}

//algorithm is borrowed from some delphi site
INL double Robot::distToWall()
{
  double p1, p2, x1, y1, x2, y2, dx, dy, Ratio, l1, l2;
  //simmilar to CRobot::sensordisttowall. gdistance is made to be as large as possible, so that first
  //  calculation would overwrite it
  double gdistance = 5000000.0, distance;
  for (int i=0; i<environment_->wallcount(); i++)
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
      distance = sqrt((p1-x1)*(p1-x1)+(p2-y1)*(p2-y1));
    else if (Ratio>1)
      distance = sqrt((p1-x2)*(p1-x2)+(p2-y2)*(p2-y2));
    else
    //if distance is hitting in the segment-wall, then distance is calculated differently
    {
      l1 = (1 - Ratio) * x1 + Ratio * x2;
      l2 = (1 - Ratio) * y1 + Ratio * y2;
      distance = sqrt((p1 - l1) * (p1 -l1) + (p2 - l2) * (p2 - l2));
    }
    if (distance < gdistance)
      gdistance = distance;
  }
  return gdistance - ROBOTS_RADIUS;
}

INL double Robot::distToPoint(double &x, double &y)
{
  double dx = position_[0] - x;
  double dy = position_[1] - y;
  return sqrt((dx)*(dx)+(dy)*(dy));
}

INL double Robot::angleToPoint(double &x, double &y, double dhead)
{
  double c = distToPoint(x, y);
  double angle = acos((x-position_[0])/c)/RAD_IN_DEG;
  if (position_[1] >= y)
    angle = -angle;
  angle -= head_ + dhead;
  angle = (angle > 180) ? angle-360 : (angle < -180) ? angle + 360 : angle;
  return angle;
}

INL void Robot::rotate(double vect[], double rot, double s)
{
  //maximum allowed turning rate at current speed
  double alfa = MAX_ROTATION_SPEEED
      - (MAX_ROTATION_SPEEED - MIN_ROTATION_SPEED) * s / MAX_SPEED;
  //current rotation speed in rads/s
  alfa *= rot * UPDATE_INTERVAL * RAD_IN_DEG;
  //x and y stands for vector to rotate coordinates
  double x = vect[0], y = vect[1];
  //rotation matrix being applied
  //[x]   [cos (alfa) -sin(alfa)]
  //[ ] × [                     ] = rotated vector by alfa
  //[y]   [sin (alfa)  cos(alfa)]
  double cosAlfa = cos(alfa);
  double sinAlfa = sin(alfa);
  vect[0] = cosAlfa * x - sinAlfa * y;
  vect[1] = sinAlfa * x + cosAlfa * y;
  //heading is updated and forced to appear in [-180;180] bounds
  head_ += rot * MAX_ROTATION_SPEEED * UPDATE_INTERVAL;
  if (head_ < -180.0)
    head_ += 360;
  else if (head_ > 180.0)
    head_ -= 360;
}

INL void Robot::newPosition(double acc2, double rot)
{
  //current speed in m/s
  double speed=sqrt(speedVector_[0]*speedVector_[0]+speedVector_[1]
      *speedVector_[1]);
  //lets rotate speed vector
  rotate(speedVector_, rot, speed);
  //acceleration vector is calculated
  double x = cos(head_*RAD_IN_DEG);
  double y = sin(head_*RAD_IN_DEG);
  double ax=x*MAX_ACCELERATION*UPDATE_INTERVAL;
  double ay=y*MAX_ACCELERATION*UPDATE_INTERVAL;
  //backing up speed vector
  double u1=speedVector_[0];
  double u2=speedVector_[1];
  //these ifs makes speedv appear closer to maxs*acc2 at maximum acceleraration speed
  //if required, lesser acceleration is used (2nd and 4th ifs)
  if ((speed/MAX_SPEED)>acc2 && (speed-MAX_SPEED*acc2)>(MAX_ACCELERATION
      *UPDATE_INTERVAL))
  {
    speedVector_[0] = x*speed-ax;
    speedVector_[1] = y*speed-ay;
  }
  else if ((speed/MAX_SPEED)>acc2 && (speed-MAX_SPEED*acc2)<=(MAX_ACCELERATION
      *UPDATE_INTERVAL))
  {
    speedVector_[0] = x*MAX_SPEED*acc2;
    speedVector_[1] = y*MAX_SPEED*acc2;
  }
  else if ((speed/MAX_SPEED)<acc2 && (MAX_SPEED*acc2-speed)>(MAX_ACCELERATION
      *UPDATE_INTERVAL))
  {
    speedVector_[0] = x*speed+ax;
    speedVector_[1] = y*speed+ay;
  }
  else if (speed==0 && acc2==0)
    ;
  else
  {
    speedVector_[0] = x*MAX_SPEED*acc2;
    speedVector_[1] = y*MAX_SPEED*acc2;
  }
  //new position is calculated
  position_[0] += (speedVector_[0] + u1)/2*UPDATE_INTERVAL;
  position_[1] += (speedVector_[1] + u2)/2*UPDATE_INTERVAL;
}
