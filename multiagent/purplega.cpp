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

#include <list>
#include "pool.cpp"

struct Population
{
    Pool* pool;
    int interval;
};

class PurpleGa
{
  public:
    PurpleGa(int, int, int, int);
    ~PurpleGa();
    void step();
    void finish();
    int getEvaluations()
    {
      return evaluations_;
    }

  private:
    void createPopulation();
    int generation_;
    int evaluations_;
    int evaluationsLastTime_;
    int base_;
    int newBarrier_;
    int newPopSize_;
    int threadCount_;
    std::list<Population> populations_;
    Environment environment_;
    Benchmarker timer_, bigTimer_;
};

PurpleGa::PurpleGa(int popSize, int base, int barrier, int threadCount)
{
  printf("\nSimulations                 Speed        Size   Fitness    Size   Fitness  ....\n\n");
  evaluations_ = 0;
  evaluationsLastTime_ = 0;
  newPopSize_ = popSize;
  generation_ = 0;
  base_ = base;
  environment_.initialise("walls.txt");
  threadCount_ = threadCount;

  createPopulation();
  newBarrier_ = barrier;
  timer_.start();
  bigTimer_.start();
}

PurpleGa::~PurpleGa()
{
  environment_.kill();
}

void PurpleGa::createPopulation()
{
  Population* temp;
  temp = new Population;
  temp->pool = new Pool;
  temp->interval = newPopSize_;

  populations_.push_back(*temp);
  populations_.back().pool->initialise(newPopSize_, &environment_, threadCount_);
  populations_.back().pool->randomizeAll();
  if (evaluations_ < 20000)
    newBarrier_ *= base_;
  else
    newBarrier_ += 20000;
}

void PurpleGa::step()
{
  ++generation_;
  if (evaluations_ > newBarrier_)
    createPopulation();

  bool shouldPrint = false;
  std::list<Population>::iterator iterator = populations_.begin();
  for (int i = 1; iterator != populations_.end(); iterator++, i *= base_)
  {
    if ((generation_ % iterator->interval == 0)
        && !iterator->pool->getIsPaused())
    {
      iterator->pool->step();
      evaluations_ += iterator->pool->getEvaluations();
      iterator->pool->resetEvaluations();
      shouldPrint = true;
      iterator->interval = iterator->pool->getPoolSize() - iterator->pool->getEliteCount();
    }
  }

  bool isAnyoneActive = false;
  for (iterator = populations_.begin(); iterator != populations_.end();)
  {
    if (!iterator->pool->getIsPaused())
    {
      isAnyoneActive = true;
      ++iterator;
    }
    else
    {
      if (iterator != populations_.begin())
      {
        std::list<Population>::iterator tempIterator = iterator;
        ++iterator;
        populations_.erase(tempIterator);
      }
      else
      {
        ++iterator;
      }
    }
  }
  if (!isAnyoneActive)
    populations_.front().pool->resetStability();

  // check if any pool needs removal
  if (populations_.size() != 1)
  {
    std::list<Population>::iterator iterator2 = populations_.begin();
    iterator2++;
    std::list<Population>::iterator iterator1 = populations_.begin();
    for (; iterator2 != populations_.end() && populations_.size() != 1;
        iterator1++, iterator2++)
    {
      if (populations_.size() > 2)
      {
        if (iterator1->pool->getBest() < iterator2->pool->getBest())
        {
          if (iterator1 != populations_.begin())
          {
            std::list<Population>::iterator tempIterator = iterator1;
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
      else if (iterator1->pool->getBest() < iterator2->pool->getBest())
      {
        populations_.erase(iterator1);
        iterator2 = populations_.end();
      }
    }
  }
  if (shouldPrint)//false)//
  {
    timer_.end();

    printf("%6i << %6i || %5.2lf ms/sim ||--||", evaluations_, newBarrier_,
        timer_.getTimeMS() / (evaluations_ - evaluationsLastTime_));
    evaluationsLastTime_ = evaluations_;

    iterator = populations_.begin();
    for (int i = 0; iterator != populations_.end() && i < 3; ++iterator, ++i)
    {
      printf(" %4i %8.2f%c ||", iterator->pool->getPoolSize(),
          iterator->pool->getBest(), (iterator->pool->getIsPaused()) ? '-'
              : '+');
    }
    printf("\n");

    timer_.start();
  }
}

void PurpleGa::finish()
{
  bigTimer_.end();
  printf("\n==================================\nSummary:\n\n");
  printf("Total simulations made: %i\n", evaluations_);
  printf("Total time: %.1f s\n", bigTimer_.getTimeS());
  printf("Average ms / simulation: %.4f ms\n", bigTimer_.getTimeMS()
      / evaluations_);
  printf("Smallest population size at the end: %i\n", populations_.front().pool->getPoolSize());

  puts("\nIntergrity:\nAll of these 4 should be equal:");
  printf("\n%f\n", populations_.front().pool->getBest());
  populations_.front().pool->score(0, false);
  printf("%f\n", populations_.front().pool->getBest());
  populations_.front().pool->score(0, false);
  printf("%f\n", populations_.front().pool->getBest());
  populations_.front().pool->score(0, true);
  printf("%f\n", populations_.front().pool->getBest());
  populations_.front().pool->outputGenes();
}
