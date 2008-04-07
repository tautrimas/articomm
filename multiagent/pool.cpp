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
#include <vector>

class Pool
{
  private:
    std::vector<PoolMember> popul_;
    int poolSize_;
    int evaluations_;
    bool isPaused_;
    int stable_;
    int stableGa_;
    Environment* environment_;
    int threadCount_;
  public:
    MiniEvolution* miniEvolution_;
    ~Pool();
    void initialise(int, Environment*, int);
    PoolMember crossover(PoolMember);
    PoolMember mutate(PoolMember, const double&);
    void mutateAll(int, double);
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
    int getEliteCount()
    {
      return (poolSize_ / 6) * 5;
    }
    void step();
    void resetStability()
    {
      stable_ = 0;
      isPaused_ = false;
    }
    void resize(int newSize);
    int hillClimbing(const int& iterations);
    int getEvaluations()
    {
      return evaluations_;
    }
    void resetEvaluations()
    {
      evaluations_ = 0;
    }
};

void Pool::initialise(int size, Environment* environment, int threadCount)
{
  popul_.reserve(size);
  PoolMember tempMember;
  popul_.assign(size, tempMember);
  poolSize_ = size;
  environment_ = environment;
  stable_ = 0;
  stableGa_ = 0;
  isPaused_ = false;
  evaluations_ = 0;
  threadCount_ = threadCount;
  miniEvolution_ = new MiniEvolution(1);
  miniEvolution_->setValue(0, size);
  miniEvolution_->setInterval(0, 6.0, 1.0e99);
  /*miniEvolution_->setValue(1, 0.08);
   miniEvolution_->setInterval(1, 0.0, 1.0);
   miniEvolution_->setValue(2, 1.0);
   miniEvolution_->setInterval(2, 0.0, 5.0);*/
  miniEvolution_->setRequiredVolume(20);
}

Pool::~Pool()
{
  delete miniEvolution_;
}

PoolMember Pool::crossover(PoolMember member)
{
  if (R.randDouble() < 0.9)
  {
    int crosspoint1 = rand() % (GENE_COUNT);
    int crosspoint2 = crosspoint1 + (rand() % (GENE_COUNT - crosspoint1));
    int member2 = rand() % getEliteCount();
    for (int i = crosspoint1; i < crosspoint2; i++)
      member.gene[i] = popul_[member2].gene[i];
  }
  return member;
}

PoolMember Pool::mutate(PoolMember member, const double& delta)
{
  double ratio = 0.05/*miniEvolution_->getValue(1)*/; //R.randdouble(0.15,1.0);
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

void Pool::mutateAll(int elite, double delta)
{
  for (int i = elite; i < poolSize_; ++i)
  {
    popul_[i] = crossover(popul_[rand() % elite]);
    popul_[i] = mutate(popul_[i], delta);
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
      //popul_[i].gene[(WEIGHT_COUNT + SENSOR_COUNT) * (j + 1) - 4] = 135.0;
      //popul_[i].gene[(WEIGHT_COUNT + SENSOR_COUNT) * (j + 1) - 3] = -135.0;
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

bool compareMembers(const PoolMember& a, const PoolMember& b)
{
  return a.fitness > b.fitness;
}

void Pool::sort()
{
  std::sort(popul_.begin(), popul_.end(), compareMembers);
}

void Pool::score(int i, bool shouldPrint)
{
  Simulation simulation(popul_[i], environment_);
  simulation.runSim(shouldPrint);
  popul_[i].fitness = simulation.getScore();
}

struct ThreadingInterval
{
    int a, b;
    Pool* pool;
};

void* scorePart(void* arg)
{
  ThreadingInterval* interval = (ThreadingInterval*)arg;
  for (int i = interval->a; i < interval->b; i++)
  {
    interval->pool->score(i, false);
  }
  pthread_exit(0);
}

void Pool::scoreAll()
{
  ThreadingInterval* intervals = new ThreadingInterval[threadCount_];
  Thread* threads = new Thread[threadCount_];

  int a = getEliteCount();
  int b = a + (poolSize_ - getEliteCount()) / threadCount_;
  for (int i = 0; i < threadCount_; ++i)
  {
    intervals[i].a = a;
    if (i < (poolSize_ - getEliteCount()) % threadCount_)
      ++b;
    intervals[i].b = b;
    intervals[i].pool = this;
    threads[i].run(scorePart, &intervals[i]);
    a = b;
    b += (poolSize_ - getEliteCount()) / threadCount_;
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
  /*if (generation_ * getPoolSize() > 25000)
   generation_++;*/
  //copy old best
  double senas = getBest();
  resize((int)miniEvolution_->getValue(0));
  int required = (int)(3000.0 / (poolSize_ + 100));
  if (required < 1)
    required = 1;
  miniEvolution_->setRequiredVolume(required/*-0.00635 * poolSize_ + 20.038*/);
  mutateAll(getEliteCount(), 2.0/*miniEvolution_->getValue(2)*/);
  scoreAll();
  evaluations_ += poolSize_ - getEliteCount();
  sort();
  miniEvolution_->addScore(senas / getBest());
  if (senas == getBest())
  {
    stable_ += poolSize_ - getEliteCount();
  }
  else
  {
    stable_ = 0;
    stableGa_ = 0;
  }
  if (stable_ > 250)
  {
    evaluations_ += hillClimbing(250);
    stable_ = 0;
  }
  if (senas == getBest())
  {
    stableGa_ += poolSize_ - getEliteCount();
  }
  if (stableGa_ > 4000)
    isPaused_ = true;
  /*if (getBest() > -150.0)
  {
    hillClimbing(1000);
  }*/
}

void Pool::resize(int newSize)
{
  if (newSize < poolSize_)
  {
    popul_.resize(newSize, popul_[1]);
    poolSize_ = newSize;
  }
  else
  {
    int oldElite = getEliteCount();
    popul_.resize(newSize, popul_[1]);
    poolSize_ = newSize;
    mutateAll(oldElite, 0.05);
  }
}

int Pool::hillClimbing(const int& iterations)
{
  bool shouldStop = false;
  int count = 0;
  for (; count < iterations && !shouldStop; ++count)
  {
    PoolMember oldMember = popul_[0];
    popul_[0] = mutate(popul_[0], 2.0);
    score(0, false);
    if (getBest() < oldMember.fitness)
      popul_[0] = oldMember;
    else
      shouldStop = true;
  }
  printf("%f\n", getBest());
  return count;
}
