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
    Pool *pool;
    bool stopit;
  public:
    CEvolution(int);
    ~CEvolution();
    void restart(int);
    bool Continue();
    bool Stop();
    void step();
    int getgeneration() {return generation;}
    double getbest() {return pool->getBest();}
    bool enough() {return stopit;}
    void finish();
};

CEvolution::CEvolution(int poolsize)
{
  generation = 1;
  stopit= false;
  pool = new Pool(poolsize);

  if (Continue())
    pool->readgenes(2.0);
  else {
    pool->randomizegenes();
  }

  pool->scoreAll();
  pool->scoreAll();
  pool->sort();

  printf("%12.2f %12.2f      1\n",pool->getBest(), pool->getBest() - pool->getMember(pool->getPoolSize()-1).fitness);
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
  pool = new Pool(poolsize);

  if (Continue())
    pool->readgenes(2.0);
  else {
    pool->randomizegenes();
  }

  pool->scoreAll();
  pool->scoreAll();
  pool->sort();

  printf("%12.2f %12.2f      1\n",pool->getBest(), pool->getBest() - pool->getMember(pool->getPoolSize()-1).fitness);
}

bool CEvolution::Continue() {
  ifstream fin("continue");
  bool testi;
  fin >> testi;
  fin.close();
  return testi;
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
  if (generation*pool->getPoolSize() > 25000)
    generation++;
  //copy old best
  double senas=pool->getBest();
  //replicate first half while mutating them and then evaluate
  pool->mutateAll(((generation*pool->getPoolSize())%10800 > 5400) ? 0.01 : 2.0);
  pool->scoreAll();
  if (generation*pool->getPoolSize() > 25000)
    pool->scoreAll();
  pool->sort();
  // output evolution progress if leader changed
  if ((senas < pool->getBest()) && (((generation*pool->getPoolSize()) % EVALUATIONS_LIMIT)!=0))
    printf("%12.2f %12.2f %6d\n",pool->getBest(), pool->getBest()-senas, generation*pool->getPoolSize());
  // output evolution progress after some time
  if (((generation*pool->getPoolSize()) % EVALUATIONS_LIMIT)==0) {
    printf("%12.2f %12.2f %6d*\n",pool->getBest(), pool->getBest()-senas, generation*pool->getPoolSize());
    stopit = Stop();
    pool->outputgenes();
  }
}

void CEvolution::finish()
{
  cout << endl;

  cout << generation << endl;
  cout <<  pool->getBest()<< endl;
  pool->score(0, false);
  cout <<  pool->getBest()<< endl;
  pool->score(0, false);
  cout <<  pool->getBest()<< endl;
  pool->score(0, false);
  cout <<  pool->getBest()<< endl;
  pool->score(0, true);
}
