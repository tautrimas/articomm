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

#include "pool.cpp"

class CEvolution {
  private:
    int generation;
    CPool *pool;
    bool stopit;
    bool contin;
  public:
    CEvolution(int);
    ~CEvolution();
    void restart(int);
    bool Continue();
    bool Stop();
    void step();
    int getgeneration() {return generation;}
    double getbest() {return pool->getbest();}
    bool getcontin() {return contin;}
    bool enough() {return stopit;}
    void finish();
};

CEvolution::CEvolution(int poolsize)
{
  generation = 1;
  stopit= false;
  contin = Continue();
  pool = new CPool(poolsize);

  if (getcontin())
    pool->readgenes(2.0);
  else {
    pool->randomizegenes();
  }

  pool->scoreall();
  pool->scoreall();
  pool->sort();

  printf("%12.2f %12.2f      1\n",pool->getbest(), pool->getbest() - pool->getmember(pool->getpoolsize()-1).fitness);
}

CEvolution::~CEvolution()
{
  delete pool;
}

void CEvolution::restart(int poolsize)
{
  delete pool;

  generation = 1;
  stopit= false;
  contin = Continue();
  pool = new CPool(poolsize);

  if (getcontin())
    pool->readgenes(2.0);
  else {
    pool->randomizegenes();
  }

  pool->scoreall();
  pool->scoreall();
  pool->sort();

  printf("%12.2f %12.2f      1*\n",pool->getbest(), pool->getbest() - pool->getmember(pool->getpoolsize()-1).fitness);
}

bool CEvolution::Continue() {
  ifstream fin("continue");
  bool contin;
  fin >> contin;
  fin.close();
  return contin;
}

bool CEvolution::Stop() {
  ifstream fin("stop", ifstream::in);
  bool stopit;
  fin >> stopit; //check if user wants to stop
  fin.close();
  return stopit;
}

void CEvolution::step()
{
  generation++;
  if (generation*pool->getpoolsize() > 25000)
    generation++;
  double senas=pool->getbest();
  pool->mutatepool(((generation*pool->getpoolsize())%10800 > 5400) ? 0.01 : 2.0);
  pool->scorepool();
  if (generation*pool->getpoolsize() > 25000)
    pool->scorepool();
  pool->sort();
  // output evolution progress if leader changed
  if ((senas < pool->getbest()) && (((generation*pool->getpoolsize()) % GEN)!=0))
    printf("%12.2f %12.2f %6d\n",pool->getbest(), pool->getbest()-senas, generation*pool->getpoolsize());
  // output evolution progress after some time
  if (((generation*pool->getpoolsize()) % GEN)==0) {
    printf("%12.2f %12.2f %6d*\n",pool->getbest(), pool->getbest()-senas, generation*pool->getpoolsize());
    stopit = Stop();
    pool->outputgenes();
  }
}

void CEvolution::finish()
{
  cout << endl;

  cout << generation << endl;
  cout <<  pool->getbest()<< endl;
  pool->score(0, false);
  cout <<  pool->getbest()<< endl;
  pool->score(0, false);
  cout <<  pool->getbest()<< endl;
  pool->score(0, false);
  cout <<  pool->getbest()<< endl;
  pool->score(0, true);
}
