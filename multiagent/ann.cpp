//     This file is part of ARTIcomm.
//
//     ARTIcomm is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     ARTIcomm is distributed in_ the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with ARTIcomm.  If not, see <http://www.gnu.org/licenses/>.

class Ann
{
  public:
    Ann(double*, double*, int, int, int);
    ~Ann();
    void sigmoid(double& node);
    void process();
    int getNodeCount()
    {
      return nodeCount_;
    }
    int getWeightCount()
    {
      return weightCount_;
    }
    void setNode(int, double);
    double getOutputNode(int);
    double getNode(int);
    void clearNodes();
    //private:
    double* nodes_;
    double* weights_;
    int in_, hid_, out_;
    int weightCount_, nodeCount_;
};

INL void Ann::setNode(int node, double value)
{
  nodes_[node] = value;
}

INL double Ann::getOutputNode(int node)
{
  return nodes_[nodeCount_ - node];
}

double Ann::getNode(int node)
{
  return nodes_[node];
}

Ann::Ann(double* weightsStart, double* weightsEnd, int in, int hid, int out)
{
  in_ = in + 1;
  hid_ = hid + 1;
  out_ = out;

  nodeCount_ = in_ + hid_ + out_;
  weightCount_ = in_ * (hid_ - 1) + hid_ * out_;

  weights_ = new double[weightCount_];
  double* iterator = weightsStart;
  for (int i = 0; iterator != weightsEnd; ++iterator, ++i)
    weights_[i] = *iterator;
  nodes_ = new double[nodeCount_];
}

Ann::~Ann()
{
  delete [] weights_;
  delete [] nodes_;
}

INL void Ann::sigmoid(double& x)
{
  /*if (nodes_[i] >= 1)
   nodes_[i] = 1;
   else
   {
   if (nodes_[i]<=-1)
   nodes_[i] = 0;
   else
   //nodes_[i] = 0.5 + nodes_[i] * (1 - fabs(nodes_[i]) / 2);
   nodes_[i] = 0.5 + 0.5 * nodes_[i];
   }
   */
  if (x < 0.5)
  {
    if (x > -0.5)
    {
      x =  0.8 * x + 0.5;
    }
    else
    {
      if (x > -1.0)
      {
        x = 0.2 * x + 0.2;
      }
      else
      {
        x = 0.0;
      }
    }
  }
  else
  {
    if (x < 1.0)
    {
      x = 0.2 * x + 0.8;
    }
    else x = 1.0;
  }
}

INL void Ann::process()
{
  for (int i = 0; i < in_; i++)
    for (int j = in_ + 1; j < in_ + hid_; j++)
      // weights_ are multiplied by nodes values and added to hidden nodes
      nodes_[j] += nodes_[i] * weights_[(j - in_ - 1) * in_ + i ];
  for (int i = in_ + 1; i < in_ + hid_; i++)
  {
    // that is a approximated (fast) sigmoid function. Resulting network[i] is in [0;1]
    // this is only hidden nodes
    sigmoid(nodes_[i]);
  }
  for (int i = in_; i < in_ + hid_; i++)
    for (int j = in_ + hid_; j < nodeCount_; j++)
      // weights_ are multiplied by nodes values and added to output nodes
      nodes_[j] += nodes_[i] * weights_[(j - in_ - hid_) * hid_ + i + in_ * ( hid_ - 2)];
  for (int i = in_ + hid_; i < nodeCount_; i++)
  {
    // again sigmoid, but this time output nodes are processed
    sigmoid(nodes_[i]);
  }
}

INL void Ann::clearNodes()
{
  for (int i = 2 + in_; i < nodeCount_; ++i)
  {
    nodes_[i] = 0;
  }
  nodes_[0] = 1;
  nodes_[in_] = 1;
}
