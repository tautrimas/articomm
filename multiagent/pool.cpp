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

#define IN 6   //number of input nodes
#define HID 4  //number of hidden nodes
#define OUT 2  //output nodes
#define WEIGHT_COUNT ((IN+1)*(HID)+(HID+1)*OUT) //number of weights
#define SENSOR_COUNT 2 //how many sensors
#define ROBOT_COUNT 5
#define GENE_COUNT ((SENSOR_COUNT + WEIGHT_COUNT) * ROBOT_COUNT)

struct PoolMember
{
    double gene[GENE_COUNT];
    double fitness;
};

#include "simulation.cpp"
#include <algorithm>
#include "benchmarker.cpp"
#include "minievolution.cpp"

class Pool
{
  private:
    PoolMember * popul_;
    int poolSize_;
    int generation_;
    bool isPaused_;
    int stable_;
    Environment* environment_;
    int threadCount_;
  public:
    MiniEvolution* miniEvolution_;
    ~Pool();
    void initialize(int, Environment*, int);
    PoolMember crossover(PoolMember);
    PoolMember mutate(PoolMember, double&);
    void mutateAll(double);
    void randomizeAll();
    /*void readgenes(double);
     void outputgenes();*/
    void sort();
    void score(int, bool);
    //void* scorePart(void*);
    void scoreAll();
    double getBest()
    {
      return popul_[0].fitness;
    }
    int getPoolSize()
    {
      return poolSize_;
    }
    PoolMember getMember(int num)
    {
      return popul_[num];
    }
    bool getIsPaused()
    {
      return isPaused_;
    }
    void step();
    void resetStability()
    {
      stable_ = 0;
      isPaused_ = false;
    }
};

void Pool::initialize(int size, Environment* environment, int threadCount)
{
  popul_ = new PoolMember[size];
  poolSize_ = size;
  environment_ = environment;
  stable_ = 0;
  isPaused_ = false;
  generation_ = 0;
  threadCount_ = threadCount;
  miniEvolution_ = new MiniEvolution(1);
  miniEvolution_->setValue(0, 0.8);
  int RequiredScores = 512 / poolSize_;
  if (RequiredScores < 1)
    RequiredScores = 1;
  miniEvolution_->setRequiredScores(RequiredScores);
}

Pool::~Pool()
{
  delete [] popul_;
  delete miniEvolution_;
}

PoolMember Pool::crossover(PoolMember member)
{
  if (R.randDouble() < miniEvolution_->getValue(0))
  {
    int crosspoint1 = rand() % (GENE_COUNT);
    int crosspoint2 = crosspoint1 + (rand() % (GENE_COUNT - crosspoint1));
    int member2 = rand() % (poolSize_ / 3);
    for (int i = crosspoint1; i < crosspoint2; i++)
      member.gene[i] = popul_[member2].gene[i];
  }
  return member;
}

PoolMember Pool::mutate(PoolMember member, double& delta)
{
  double ratio = 0.1; //R.randdouble(0.15,1.0);
  for (int i = 0; i < GENE_COUNT; i++)
  {
    if (R.randDouble() < ratio)
      member.gene[i] += R.randDouble(-member.gene[i] * delta, member.gene[i]
          * delta);
  }
  /*for (int i=wcount;i<genecount;i++)
   member.gene[i] += R.randdouble(-1.0,1.0);*/
  return member;
}

void Pool::mutateAll(double delta)
{
  for (int i = (poolSize_ / 3); i < poolSize_ / 3 * 2; i++)
  {
    popul_[i] = crossover(popul_[i - (poolSize_ / 3)]);
    popul_[i] = mutate(popul_[i - (poolSize_ / 3)], delta);
    popul_[i].fitness = -9999.0;
  }
  for (int i = ((poolSize_ / 3) * 2); i < poolSize_; i++)
  {
    popul_[i] = crossover(popul_[i - (poolSize_ / 3)]);
    popul_[i] = mutate(popul_[i - ((poolSize_ / 3) * 2)], delta);
    popul_[i].fitness = -9999.0;
  }
}

void Pool::randomizeAll()
{
  for (int i = 0; i < poolSize_; i++)
  {
    for (int j = 0; j < ROBOT_COUNT; ++j)
    {
      for (int k = 0; k < WEIGHT_COUNT; k++)
      {
        //we use gaussian funcion to make more values closer to zero and some closer to 1.
        //double x = R.randDouble(-2, 2);
        popul_[i].gene[(WEIGHT_COUNT + SENSOR_COUNT) * j + k] = R.randDouble(-2.0, 2.0);
        //1 * exp(-x * x);
      }
      /*for (int j=wkiekis;j<wkiekis+sensorcount;j++)
       popul[i].gene[j]=(j-wkiekis)*110.0-55.0;//R.randdouble(-180,180);*/
      popul_[i].gene[(WEIGHT_COUNT + SENSOR_COUNT) * (j + 1) - 2] = -55.0;
      popul_[i].gene[(WEIGHT_COUNT + SENSOR_COUNT) * (j + 1) - 1] = 55.0;
    }
    popul_[i].fitness = -9999.0;
  }
}

/*void CPool::readgenes(double delta) {
 ifstream fin("weights.txt", ifstream::in);
 for (int i=0;i<poolsize/3;i++) {
 for (int j=0;j<genecount;j++) {
 fin >> popul[i].gene[j];
 }
 }
 fin.close();
 mutatepool(delta);
 }*/

/*void CPool::outputgenes()
 {
 FILE *wout;
 wout = fopen("weights.txt","w");
 for (int i=0;i<poolsize/3;i++) {
 for (int j=0;j<genecount;j++) fprintf (wout,"%0.15f\n",popul[i].gene[j]);
 fprintf (wout,"\n");
 }
 fclose(wout);
 }*/

bool compareMembers(PoolMember a, PoolMember b)
{
  return a.fitness > b.fitness;
}

void Pool::sort()
{
  std::sort(popul_, popul_ + poolSize_, compareMembers);
}

void Pool::score(int i, bool shouldPrint)
{
  Simulation simulation(popul_[i], environment_);
  simulation.runSim(shouldPrint);
  popul_[i].fitness = simulation.getScore();
}

struct Interval
{
    int a, b;
    Pool* pool;
};

void* scorePart(void* arg)
{
  Interval* interval = (Interval*)arg;
  for (int i = interval->a; i < interval->b; i++)
  {
    interval->pool->score(i, false);
  }
  pthread_exit(0);
}

void Pool::scoreAll()
{
  Interval* intervals = new Interval[threadCount_];
  Thread* threads = new Thread[threadCount_];

  int a = poolSize_ / 3;
  int b = ((poolSize_ / 3) * 2) / threadCount_;
  for (int i = 0; i < threadCount_; ++i)
  {
    intervals[i].a = a;
    if (i < ((poolSize_ / 3) * 2) % threadCount_)
      ++b;
    intervals[i].b = b;
    intervals[i].pool = this;
    threads[i].run(scorePart, &intervals[i]);
    a = b;
    b += ((poolSize_ / 3) * 2) / threadCount_;
  }
  for (int i = 0; i < threadCount_; ++i)
  {
    threads[i].join();
  }
  delete [] intervals;
  delete [] threads;
}

void Pool::step()
{
  ++generation_;
  /*if (generation_ * getPoolSize() > 25000)
   generation_++;*/
  //copy old best
  double senas = getBest();
  //replicate first half while mutating them and then evaluate
  mutateAll(( (generation_ % 4) > 0) ? 0.01 : 2.0);
  scoreAll();
  sort();
  if (senas == getBest())
    ++stable_;
  else
    stable_ = 0;
  if (stable_ * poolSize_ > 20000)
    isPaused_ = true;
  miniEvolution_->addScore(senas / getBest());
}
