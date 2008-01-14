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

class CRobot {
  public:
    CRobot(CEnvironment);
    ~CRobot() {}
    void rotate(double vect[],double rot,double s);  //rotates speed vector. needs desired rotation speed and current speed
    void newposition(double, double);  //calculates new osition. needs acceleration (0 to 1) and rotaion speed (0 to 1)
    double sensordisttowall(double dhead);  //returns sensor value (0 to 1). 1 is equals maxsensdist or more
    double disttowall();  //returns distance in metres to nearest wall
    double disttopoint(double x, double y);
    double angletopoint(double x, double y, double dhead); //returns angle to a point when dhead is angle from heading

    double speedv[2];  //speedvector
    double head;  // heading starting from east and increasing counter-clockwise
    double position[2];  //position
    double hunger;  //hunger level
    double eating;  //eating state
    double sleep;
    double sleeping;
  private:
    CEnvironment envir;
};

CRobot::CRobot(CEnvironment environment) {
  position[0] = startx;
  position[1] = starty;
  speedv[0] = 0;
  speedv[1] = 0;
  head = startrot;
  hunger = 0.0;  //not hungry
  eating = 0; //not eating
  sleep = 0.3; //not sleepy
  sleeping = 0; //not sleeping
  envir = environment;
}

double CRobot::sensordisttowall(double dhead) {
  double a1[2], b1[2], b2[2], B[2], v1[2], v2[2], det11, det12, det2, t1, t2;
  //absolute heading of a sensor
  double newhead = head + dhead;
  //a2 and a1 is calculated. Then we have v1 which is vector of a sensor. Its length is spindulys.
  a1[0] = position[0];
  a1[1] = position[1];
  v1[0] = cos(newhead*deginrad)*spindulys;
  v1[1] = sin(newhead*deginrad)*spindulys;
  //prepearing to calculate shortest distance to wall
  double gdistance = 1, distance;
  for (int i=0;i<envir.wallcount();i++) {
    //wall's b1, b2 and v2, where v2 is walls length
    b1[0] = envir.getcorner(i,0);
    b1[1] = envir.getcorner(i,1);
    b2[0] = envir.getcorner(i,2);
    b2[1] = envir.getcorner(i,3);
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
        distance = t1-1;
        //if distance is more than sensor can see
        if (distance >= maxsensdist) distance = 1;
        else {
          //if robot is hitting a wall
          if (distance <= 0) distance = 0; else {
            //distance is finally double between 0 and 1 if it is in range
            distance /= maxsensdist;
          }
        }
      }
      else distance = 1;
    }
    //if wall and sensor are parallel
    else distance = 1;
    //check if this is the smallest distance
    if (distance < gdistance) gdistance = distance;
  }
  //returns global distance as double between 0 and 1
  return gdistance;
}

//algorithm is borrowed from some delphi site
double CRobot::disttowall() {
  double p1,p2,x1,y1,x2,y2,dx,dy,Ratio,l1,l2;
  //simmilar to CRobot::sensordisttowall. gdistance is made to be as large as possible, so that first
  //  calculation would overwrite it
  double gdistance = 5000000.0, distance;
  for (int i=0;i<envir.wallcount();i++) {
    p1=position[0];
    p2=position[1];
    x1=envir.getcorner(i,0);
    y1=envir.getcorner(i,1);
    x2=envir.getcorner(i,2);
    y2=envir.getcorner(i,3);
    dx = x2 - x1;
    dy = y2 - y1;
    //ratio is the place, where distance from robot is hitting the segment
    Ratio = ((p1 - x1) * dx + (p2 - y1) * dy) / (dx * dx + dy * dy);
    //if distance is hitting not in segment, distance is calculated by simple pitagorian rules
    if (Ratio<0) distance = sqrt((p1-x1)*(p1-x1)+(p2-y1)*(p2-y1)); else
          if (Ratio>1) distance = sqrt((p1-x2)*(p1-x2)+(p2-y2)*(p2-y2)); else
                //if distance is hitting in the segment-wall, then distance is calculated differently
          {
            l1 = (1 - Ratio) * x1 + Ratio * x2;
            l2 = (1 - Ratio) * y1 + Ratio * y2;
            distance = sqrt((p1-l1)*(p1-l1)+(p2-l2)*(p2-l2));
          }
          if (distance < gdistance) gdistance = distance;
  }
  return gdistance;
}

double CRobot::disttopoint(double x, double y) {
  double c = sqrt((position[0]-x)*(position[0]-x)+(position[1]-y)*(position[1]-y));
  return c;
}

double CRobot::angletopoint(double x, double y, double dhead) {
  double c = disttopoint(x,y);
  double angle;
  if (position[1] < y) angle = acos((x-position[0])/c)/deginrad;
  else angle = -acos((x-position[0])/c)/deginrad;
  angle -= head+dhead;
  angle = (angle>180) ? angle-360 : (angle<-180) ? angle+360 : angle;
  return angle;
}

void CRobot::rotate(double vect[],double rot,double s) {
  //maximum allowed turning rate at current speed
  double alfa=maxrot-(maxrot-minrot)*s/maxs;
  //current rotation speed in rads/s
  alfa *= rot * t * deginrad;
  //x and y stands for vector to rotate coordinates
  double x=vect[0],y=vect[1];
  //rotation matrix being applied
  //[x]   [cos (alfa) -sin(alfa)]
  //[ ] × [                     ] = rotated vector by alfa
  //[y]   [sin (alfa)  cos(alfa)]
  vect[0] = cos(alfa) * x - sin(alfa) * y;
  vect[1] = sin(alfa) * x + cos(alfa) * y;
  //heading is updated and forced to appear in [-180;180] bounds
  head += rot*maxrot*t;
  if (head < -180.0) head+=360; else
        if (head > 180.0) head-=360;
}

void CRobot::newposition(double acc2, double rot) {
  //current speed in m/s
  double speed=sqrt(speedv[0]*speedv[0]+speedv[1]*speedv[1]);
  //lets rotate speed vector
  rotate(speedv,rot,speed);
  //acceleration vector is calculated
  double x = cos(head*deginrad);
  double y = sin(head*deginrad);
  double ax=x*maxacc*t;
  double ay=y*maxacc*t;
  //backing up speed vector
  double u1=speedv[0];
  double u2=speedv[1];
  //these ifs makes speedv appear closer to maxs*acc2 at maximum acceleraration speed
  //if required, lesser acceleration is used (2nd and 4th ifs)
  if ((speed/maxs)>acc2 && (speed-maxs*acc2)>(maxacc*t)) {
    speedv[0] = x*speed-ax;
    speedv[1] = y*speed-ay;
  }
  else if ((speed/maxs)>acc2 && (speed-maxs*acc2)<=(maxacc*t)) {
    speedv[0] = x*maxs*acc2;
    speedv[1] = y*maxs*acc2;
  }
  else if ((speed/maxs)<acc2 && (maxs*acc2-speed)>(maxacc*t)) {
    speedv[0] = x*speed+ax;
    speedv[1] = y*speed+ay;
  }
  else if (speed==0 && acc2==0);
  else {
    speedv[0] = x*maxs*acc2;
    speedv[1] = y*maxs*acc2;
  }
  //new position is calculated
  position[0] += (speedv[0] + u1)/2*t;
  position[1] += (speedv[1] + u2)/2*t;
}
