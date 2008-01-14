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

class CPool {
  private:
    TPopul * popul;
    int poolsize;
    CEnvironment environment;
  public:
    CPool(int);
    ~CPool();
    TPopul crossover(TPopul);
    TPopul mutate(TPopul, double);
    TPopul mutate2(TPopul, double);
    void mutatepool(double);
    void randomizegenes();
    void readgenes(double);
    void outputgenes();
    void sort();
    void score(int, bool);
    void scorepool();
    double getbest() {return popul[0].fitness;}
    int getpoolsize() {return poolsize;}
    TPopul getmember(int num) {return popul[num];}

};

CPool::CPool(int size)
{
  popul = new TPopul[size];
  poolsize = size;
  environment.initwalls();
}

CPool::~CPool()
{
  delete [] popul;
  environment.killwalls();
}

TPopul CPool::crossover(TPopul member)
{
  if ((rand() % 5) < 4)
  {
    int crosspoint1 = rand() % (genecount);
    int crosspoint2 = crosspoint1+(rand() % (genecount-crosspoint1));
    int member2 = rand() % (poolsize/3);
    for (int i=crosspoint1;i<crosspoint2;i++)
      member.gene[i] = popul[member2].gene[i];
  }
  return member;
}

TPopul CPool::mutate(TPopul member, double delta)
{
  double ratio = 0.6; //R.randdouble(0.15,1.0);
  ratio *= ratio;
  /*for (int i=0;i<wkiekis;i++)
    //simple mutation
  if (R.randdouble() < ratio)
  member.gene[i] = R.randdouble(-amplitude,amplitude)+R.randdouble()/1000000;*/
  for (int i=0;i<genecount-2;i++)
    //simple mutation
    if (R.randdouble() < ratio)
      member.gene[i] += R.randdouble(-member.gene[i]*delta,member.gene[i]*delta);

  /*for (int i=wcount1+wcount2;i<genecount;i++)
    member.gene[i] += R.randdouble(-1.0,1.0);*/
  member.fitness=0;
  return member;
}

TPopul CPool::mutate2(TPopul member, double delta)
{
  double ratio = 0.6; //R.randdouble(0.15,1.0);
  ratio *= ratio;
  /*for (int i=0;i<wkiekis;i++)
    //simple mutation
  if (R.randdouble() < ratio)
  member.gene[i] = R.randdouble(-amplitude,amplitude)+R.randdouble()/1000000;*/
  for (int i=0;i<wcount1;i++)
    //simple mutation
    if (R.randdouble() < ratio)
      member.gene[i] += R.randdouble(-member.gene[i]*delta,member.gene[i]*delta);

  /*for (int i=wcount1+wcount2;i<genecount;i++)
  member.gene[i] += R.randdouble(-1.0,1.0);*/
  member.fitness=0;
  return member;
}

void CPool::mutatepool(double delta)
{
  for (int i=(poolsize / 3);i<poolsize/3*2;i++) {
    popul[i] = crossover(popul[i-(poolsize/3)]);
    popul[i] = mutate2(popul[i-(poolsize/3)],delta);
  }
  for (int i=((poolsize / 3) * 2);i<poolsize;i++) {
    popul[i] = crossover(popul[i-(poolsize/3)]);
    popul[i] = mutate2(popul[i-((poolsize/3)*2)],delta);
  }
}

void CPool::randomizegenes()
{
  double x;
  for (int i=0;i<poolsize;i++) {
    for (int j=0;j<wcount1+wcount2;j++)
    {
      //we use gaussian funcion to make more values closer to zero and some closer to 1.
      x = R.randdouble(-2,2);
      popul[i].gene[j]=1000*exp(-x*x);
    }
    /*for (int j=wkiekis;j<wkiekis+sensorcount;j++)
    popul[i].gene[j]=(j-wkiekis)*110.0-55.0;//R.randdouble(-180,180);*/
    popul[i].gene[genecount-2] = -55.0;
    popul[i].gene[genecount-1] = 55.0;
  }
}

void CPool::readgenes(double delta) {
  ifstream fin("weights.txt", ifstream::in);
  for (int i=0;i<poolsize/3;i++) {
    for (int j=0;j<genecount;j++) {
      fin >> popul[i].gene[j];
    }
  }
  fin.close();
  mutatepool(delta);
}

void CPool::outputgenes()
{
  FILE *wout;
  wout = fopen("weights.txt","w");
  for (int i=0;i<poolsize/3;i++) {
    for (int j=0;j<genecount;j++) fprintf (wout,"%0.15f\n",popul[i].gene[j]);
    fprintf (wout,"\n");
  }
  fclose(wout);
}

void CPool::sort()
{
  int n=poolsize, j;
  int h=1, nm=n / 24;
  TPopul a;
  while  (h < nm) h = 2 * h + 1;
  do {
    for (int i=h+1; i<=n; i++) {
      a = popul[i-1];
      j = i;
      while ( (j>h) && (a.fitness>popul[j-h-1].fitness) ) {
        popul[j-1] = popul[j-h-1];
        j -= h;
      }
      popul[j-1] = a;
    }
    h = h / 2;
  } while (!(h<1));
}

void CPool::score(int i, bool print)
{
  /*if (true)
  {
    CSimulation simulation(popul[i],environment);
    simulation.runsim2();
    popul[i].fitness = simulation.getscore();
  }
  else*/
  {
    CSimulation simulation(popul[i],environment);
    simulation.runsim(print);
    popul[i].fitness = simulation.getscore();
  }
}

void CPool::scorepool()
{

  for (int i=0;i<poolsize;i++) score(i, false);
}
