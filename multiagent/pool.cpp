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

#include "simulation.cpp"
#include <algorithm>

class Pool {
  private:
    PoolMember * popul_;
    int poolSize_;
    int generation_;
    bool isPaused_;
    int stable_;
    Environment* environment_;
  public:
    Pool() {}
    ~Pool();
    void initialize(int, Environment*);
    PoolMember crossover(PoolMember);
    PoolMember mutate(PoolMember, double&);
    void mutateAll(double);
    void randomizeAll();
    /*void readgenes(double);
    void outputgenes();*/
    void sort();
    void score(int, bool);
    void scoreAll();
    double getBest() {return popul_[0].fitness;}
    int getPoolSize() {return poolSize_;}
    PoolMember getMember(int num) {return popul_[num];}
    bool getIsPaused() { return isPaused_;}
    void step();
    void resetStability() { stable_ = 0; isPaused_ = false; }
};

void Pool::initialize(int size, Environment* environment)
{
  popul_ = new PoolMember[size];
  poolSize_ = size;
  environment_ = environment;
  stable_ = 0;
  isPaused_ = false;
  generation_ = 0;
}

Pool::~Pool()
{
  delete [] popul_;
}

PoolMember Pool::crossover(PoolMember member)
{
  if ((rand() % 5) < 4)
  {
    int crosspoint1 = rand() % (GENE_COUNT);
    int crosspoint2 = crosspoint1+(rand() % (GENE_COUNT-crosspoint1));
    int member2 = rand() % (poolSize_ / 3);
    for (int i = crosspoint1; i < crosspoint2; i++)
      member.gene[i] = popul_[member2].gene[i];
  }
  return member;
}

PoolMember Pool::mutate(PoolMember member, double& delta)
{
  double ratio = 1.1; //R.randdouble(0.15,1.0);
  for (int i = 0; i < GENE_COUNT - 2; i++)
  {
    //simple mutation
    if (R.randDouble() < ratio)
      member.gene[i] += R.randDouble(-member.gene[i] * delta, member.gene[i] * delta);
  }
  /*for (int i=wcount;i<genecount;i++)
    member.gene[i] += R.randdouble(-1.0,1.0);*/
  member.fitness = -1000.0;
  return member;
}

void Pool::mutateAll(double delta)
{
  for (int i = (poolSize_ / 3); i < poolSize_ / 3 * 2; i++) {
    popul_[i] = crossover(popul_[i - (poolSize_ / 3)]);
    popul_[i] = mutate(popul_[i - (poolSize_ / 3)], delta);
  }
  for (int i=((poolSize_ / 3) * 2);i<poolSize_;i++) {
    popul_[i] = crossover(popul_[i - (poolSize_ / 3)]);
    popul_[i] = mutate(popul_[i - ((poolSize_ / 3) * 2)], delta);
  }
}

void Pool::randomizeAll()
{
  for (int i = 0;i < poolSize_; i++) {
    for (int j = 0; j < WEIGHT_COUNT; j++)
    {
      //we use gaussian funcion to make more values closer to zero and some closer to 1.
      double x = R.randDouble(-2, 2);
      popul_[i].gene[j] = 1000 * exp(-x * x);
    }
    /*for (int j=wkiekis;j<wkiekis+sensorcount;j++)
    popul[i].gene[j]=(j-wkiekis)*110.0-55.0;//R.randdouble(-180,180);*/
    popul_[i].gene[GENE_COUNT - 2] = -55.0;
    popul_[i].gene[GENE_COUNT - 1] = 55.0;
    popul_[i].fitness = -1000.0;
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
  /*int n=poolSize_, j;
  int h=1, nm=n / 24;
  PoolMember a;
  while  (h < nm) h = 2 * h + 1;
  do {
    for (int i=h+1; i<=n; i++) {
      a = popul_[i-1];
      j = i;
      while ( (j>h) && (a.fitness>popul_[j-h-1].fitness) ) {
        popul_[j-1] = popul_[j-h-1];
        j -= h;
      }
      popul_[j-1] = a;
    }
    h = h / 2;
  } while (!(h<1));*/
  std::sort(popul_, popul_ + poolSize_, compareMembers);
}

void Pool::score(int i, bool shouldPrint)
{
  Simulation simulation(popul_[i], environment_);
  simulation.runSim(shouldPrint);
  popul_[i].fitness = simulation.getScore();
}

void Pool::scoreAll()
{
  for (int i = 0; i < poolSize_; i++) score(i, false);
}

void Pool::step()
{
  generation_++;
  if (generation_ * getPoolSize() > 25000)
    generation_++;
  //copy old best
  double senas = getBest();
  //replicate first half while mutating them and then evaluate
  mutateAll(( (generation_ * getPoolSize()) % 4000 > 2000) ? 0.01 : 2.0);
  scoreAll();
  if (generation_ * getPoolSize() > 25000)
    scoreAll();
  sort();
  /*
  // output evolution progress if leader changed
  if ((senas < pool->getbest()) && (((generation*pool->getpoolsize()) % GEN)!=0))
    printf("%12.2f %12.2f %6d\n",pool->getbest(), pool->getbest()-senas, generation*pool->getpoolsize());
  // output evolution progress after some time
  if (((generation*pool->getpoolsize()) % GEN)==0) {
    printf("%12.2f %12.2f %6d*\n",pool->getbest(), pool->getbest()-senas, generation*pool->getpoolsize());
    stopit = Stop();
    pool->outputgenes();
  */

  if (senas >= getBest() || (getBest() - senas < getBest() * 0.005))
    ++stable_;
  if (stable_ * poolSize_ > 20000)
    isPaused_ = true;
}
