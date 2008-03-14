#include <list>
#include "pool.cpp"

struct Population
{
    Pool pool();
};

class Plga
{
  public:
    Plga(int, int, int);
    ~Plga();
    void step();
    void finish();

  private:
    void createPopulation();
    int generation_;
    int evaluations_;
    int base_;
    int newBarrier_;
    int newPopSize_;
    list<Pool> populations_;
    Environment environment_;
};

Plga::Plga(int popSize, int base, int barrier)
{
  evaluations_ = 0;
  newPopSize_ = popSize;
  generation_ = 1;
  base_ = base;
  environment_.initializeWalls();

  createPopulation();
  newBarrier_ = barrier;
}

Plga::~Plga()
{
  environment_.killWalls();
}

void Plga::createPopulation()
{
  Pool* temp;
  temp = new Pool;
  populations_.push_back(*temp);
  populations_.back().initialize(newPopSize_, &environment_);
  populations_.back().randomizeAll();
  evaluations_ += newPopSize_;
  newPopSize_ *= 2;
  newBarrier_ *= base_;
}

void Plga::step()
{
  ++generation_;
  if (evaluations_ > newBarrier_)
    createPopulation();

  bool shouldPrint = false;
  list<Pool>::iterator iterator = populations_.begin();
  for (int i = 1; iterator != populations_.end(); iterator++, i *= base_)
  {
    if (((generation_ % i == 0) && !(iterator->getIsPaused()))
        || populations_.size() == 1)
    {
      iterator->step();
      evaluations_ += iterator->getPoolSize();
      shouldPrint = true;
    }
  }

  bool isAnyoneActive = false;
  for (iterator = populations_.begin(); iterator != populations_.end(); )
  {
    if (!iterator->getIsPaused())
    {
      isAnyoneActive = true;
      ++iterator;
    }
    /*else
    {
      if (iterator != populations_.begin())
      {
        list<Pool>::iterator tempIterator = iterator;
        ++iterator;
        populations_.erase(tempIterator);
      }
    }*/
  }
  if (!isAnyoneActive)
    populations_.front().resetStability();

  // check if any pool needs removal
  if (populations_.size() != 1)
  {
    list<Pool>::iterator iterator2 = populations_.begin();
    iterator2++;
    list<Pool>::iterator iterator1 = populations_.begin();
    for (; iterator2 != populations_.end() && populations_.size() != 1;
        iterator1++, iterator2++)
    {
      if (populations_.size() > 2)
      {
        if (iterator1->getBest() <= iterator2->getBest())
        {
          if (iterator1 != populations_.begin())
          {
            list<Pool>::iterator tempIterator = iterator1;
            tempIterator--;
            populations_.erase(iterator1);
            iterator1 = tempIterator;
          }
          else
          {
            populations_.erase(iterator1);
            iterator2++;
            iterator1 = populations_.begin();
          }
        }
      }
      else if (iterator1->getBest() <= iterator2->getBest())
      {
        populations_.erase(iterator1);
        iterator2 = populations_.end();
      }
    }
  }
  if (shouldPrint)
  {
    printf("%6i %10i   --", generation_, evaluations_);
    for (iterator = populations_.begin(); iterator != populations_.end(); iterator++)
      printf(" %4i %10.2f%c", iterator->getPoolSize(), iterator->getBest(),
          (iterator->getIsPaused()) ? '-' : '+');
    printf(" %i\n", newBarrier_);
  }
}

void Plga::finish()
{
  printf("\n%f\n", populations_.front().getBest());
  populations_.front().score(0, false);
  printf("%f\n", populations_.front().getBest());
  populations_.front().score(0, false);
  printf("%f\n", populations_.front().getBest());
  populations_.front().score(0, true);
  printf("%f\n", populations_.front().getBest());
}
