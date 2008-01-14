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

class CSimulation {
  private:
    TPopul member;
    CEnvironment environment;
  public:
    CSimulation(TPopul &member2, CEnvironment &environment2) {member = member2; environment = environment2;}
    ~CSimulation() {}
    void runsim(bool);
    double getscore() {return member.fitness;}
    void preparesimulationfile();
    void outputsimulationstep(double, double, double, double, double, double, double, double, double);
};

void CSimulation::preparesimulationfile() {
  FILE *wout;
  wout = fopen("coords.txt","w");
  fclose(wout);
}

void CSimulation::outputsimulationstep(double a1, double a2, double a3, double a4, double a5, double a6, double a7, double a8, double a9) {
  FILE *wout;
  wout = fopen("coords.txt","a");
  fprintf (wout,"%0.15f %0.15f %0.15f %0.15f %0.15f %0.15f %0.15f %0.15f %0.15f\n",a1,a2,a3,a4,a5,a6,a7,a8,a9);
  fclose(wout);
}

void CSimulation::runsim(bool printprogress)
{
  //robot initialization
  CRobot robot(environment);

  if (printprogress) preparesimulationfile();
  //node arrays are being created
  double mas1[IN1+HID1+OUT1],mas2[IN2+HID2+OUT2];
  mas1[0]=1;
  mas1[IN1]=1;
  mas2[0]=1;
  mas2[IN2]=1;
  //read weights from genotype
  double weights1[wcount1];
  for (int i=0;i<wcount1;i++)
    weights1[i] = member.gene[i];

  double weights2[wcount2];
  for (int i=wcount1;i<wcount1+wcount2;i++)
    weights2[i-wcount1] = member.gene[i];

  CANN ann1(weights1, IN1, HID1, OUT1), ann2(weights2, IN2, HID2, OUT2);
  //fittnes is starting from zero
  float fit = 0;
  //step*t is the second being virtualized
  int step = 0;
  //variable for distance to nearest wall for collision detection
  double atst;
  int stop = 0;
  int voyages = 0; // times robot went from one corner to another
  //virtualization begins
  do {
    step++;
    //distance to nearest wall is calculated for collision detection
    atst = robot.disttowall()-spindulys;
    if (atst < 0) {
      //if crash, then terminate
      //fit += step*t*800;
      fit -= 2000;
      stop = 1;
    }

    robot.hunger += (1.0/sectostarve)*t;
    //if not on feeding area, then feel increasing hunger
    if (!(robot.position[0] > 0.7 && robot.position[1] < -0.4)) {
      robot.eating = 0;
      if (robot.hunger >= 1) {
        fit -= 1000;
        //fit += step*t*800;
        stop = 1;
      }
    }
    //else, feed yourself and don't feel hungry
    else {
      if (robot.hunger > 0) robot.eating+=(1.0/2.0)*t;
      else robot.hunger = 0;
      if (robot.eating > 1) {
        if (robot.sleep<robot.hunger) {
          fit += 500;
          voyages++;
        }
        robot.hunger = 0;
        robot.eating = 0;
      }
    }

    robot.sleep += (1.0/sectosleep)*t;
    //if not on sleeping area, then feel increasing need for sleep
    if (!(robot.position[0] < 0.2 && robot.position[1] > -0.2)) {
      robot.sleeping = 0;
      if (robot.sleep >= 1) {
        fit -= 1000;
        //fit += step*t*800;
        stop = 1;
      }
    }
    //else, sleep
    else {
      if (robot.sleep > 0) robot.sleeping+=(1.0/2.0)*t;
      else robot.sleep = 0;
      if (robot.sleeping > 1) {
        if (robot.sleep>robot.hunger) {
          fit += 500;
          voyages++;
        }
        robot.sleep = 0;
        robot.sleeping = 0;
      }
    }
    //all sensor states are being updated
    for (int i=0;i<sensorcount;i++)
      //their states are transmited to ANN inputs
      mas1[i+1] = robot.sensordisttowall(member.gene[wcount1+wcount2+i]);

    mas2[1] = robot.angletopoint(0.0,0.0,0.0);
    mas2[2] = robot.angletopoint(0.9,-0.6,0.0);
    mas2[3] = (robot.sleeping>0.001) ? 1 : 0;
    mas2[4] = (robot.eating>0.001) ? 1 : 0;
    mas2[5] = robot.sleep;
    mas2[6] = robot.hunger;

    fit += (1-robot.hunger)*100*t;
    fit += (1-robot.sleep)*100*t;



    //process ANNs with given inputs
    ann2.process(mas2);
    mas1[sensorcount+1] = mas2[IN2+HID2+OUT2-2];
    mas1[sensorcount+2] = mas2[IN2+HID2+OUT2-1];
    ann1.process(mas1);

    //new position is counted. ANN's outputs are acceleration and rotation speed.
    //values from ANN are from 0 to 1 so they must be converted to [-1;1] for normal operation
    robot.newposition(mas1[IN1+HID1+OUT1-2],(mas1[IN1+HID1+OUT1-1]-0.5)*2);
    if (printprogress) {
      outputsimulationstep(robot.position[0],robot.position[1],robot.head,robot.eating,robot.sleeping,robot.hunger,robot.sleep,mas2[1],t*step);
    }
  } while (step*t<roundl && !stop); //while maximum round length is not exceeded

  double tvoyages = robot.disttopoint(((robot.sleep>robot.hunger) ? 0 : 0.9),((robot.sleep>robot.hunger) ? 0 : -0.6)) * cos((((robot.sleep>robot.hunger) ? mas2[1]+robot.head : mas2[2]+robot.head)  +  ((robot.sleep<robot.hunger) ? 1 : -1)*90  -  atan(0.6/0.9)/deginrad  )*deginrad);

  fit += (voyages+1-tvoyages)*8000;
  member.fitness = fit;
}
