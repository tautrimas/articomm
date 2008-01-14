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

class CANN {
  private:
    double *weights;
    int in, hid, out;
  public:
    CANN(double*, int, int, int);
    ~CANN();
    void process(double*);
    int getnodecount() {return in+hid+out;}
    int getweightcount() {return (in*(hid-1)+hid*out);}
};

CANN::CANN(double wei[], int in2, int hid2, int out2)
{
  in = in2;
  hid = hid2;
  out = out2;
  weights = new double[getweightcount()];
  for (int i=0;i<getweightcount();i++)
    weights[i] = wei[i];
}

CANN::~CANN()
{
  delete [] weights;
}

void CANN::process(double network[]) {
  for (int i=0;i<in;i++)
    for (int j=in+1;j<in+hid;j++)
      //weights are multiplied by nodes values and added to hidden nodes
      network[j] += network[i] * weights[(j-in-1)*in+i];
  for (int i=in+1;i<in+hid;i++) {
    //that is a approximated (fast) sigmoid function. Resulting network[i] is in [0;1]
    //this is only hidden nodes
    if (network[i]>=1) network[i] = 1; else
          if (network[i]<=-1) network[i] = 0; else
                network[i] = 0.5 + network[i] * (1 - fabs(network[i]) / 2);
  }
  for (int i=in;i<in+hid;i++)
    for (int j=in+hid;j<in+hid+out;j++)
      //weights are multiplied by nodes values and added to output nodes
      network[j] += network[i] * weights[(j-in-hid)*hid+i+in*(hid-2)];
  for (int i=in+hid;i<in+hid+out;i++) {
    //again sigmoid, but this time output nodes are processed
    if (network[i]>=1) network[i] = 1; else
          if (network[i]<=-1) network[i] = 0; else
                network[i] = 0.5 + network[i] * (1 - fabs(network[i]) / 2);
  }
}
