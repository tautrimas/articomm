#include <list>
#include "pool.cpp"

struct Population
{
    Pool pool();
};

class Plga
{
  public:
    Plga(int, int, int, int);
    ~Plga();
    void step();
    void finish();

  private:
    void createPopulation();
    int generation_;
    int evaluations_;
    int evaluationsLastTime_;
    int base_;
    int newBarrier_;
    int newPopSize_;
    int threadCount_;
    std::list<Pool> populations_;
    Environment environment_;
    Benchmarker timer_, bigTimer_;
};

Plga::Plga(int popSize, int base, int barrier, int threadCount)
{
  printf("\nSimulations                 Speed        Size   Fitness    Size   Fitness  ....\n\n");
  evaluations_ = 0;
  evaluationsLastTime_ = 0;
  newPopSize_ = popSize;
  generation_ = 1;
  base_ = base;
  environment_.initializeWalls();

  createPopulation();
  newBarrier_ = barrier;
  timer_.start();
  bigTimer_.start();

  threadCount_ = threadCount;
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
  populations_.back().initialize(newPopSize_, &environment_, threadCount_);
  populations_.back().randomizeAll();
  newPopSize_ *= 2;
  newBarrier_ *= base_;
}

void Plga::step()
{
  ++generation_;
  if (evaluations_ > newBarrier_)
    createPopulation();

  bool shouldPrint = false;
  std::list<Pool>::iterator iterator = populations_.begin();
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
  for (iterator = populations_.begin(); iterator != populations_.end();)
  {
    if (!iterator->getIsPaused())
    {
      isAnyoneActive = true;
      ++iterator;
    }
    else
    {
      if (iterator != populations_.begin())
      {
        std::list<Pool>::iterator tempIterator = iterator;
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
    populations_.front().resetStability();

  // check if any pool needs removal
  if (populations_.size() != 1)
  {
    std::list<Pool>::iterator iterator2 = populations_.begin();
    iterator2++;
    std::list<Pool>::iterator iterator1 = populations_.begin();
    for (; iterator2 != populations_.end() && populations_.size() != 1;
        iterator1++, iterator2++)
    {
      if (populations_.size() > 2)
      {
        if (iterator1->getBest() <= iterator2->getBest())
        {
          if (iterator1 != populations_.begin())
          {
            std::list<Pool>::iterator tempIterator = iterator1;
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
    timer_.end();

    printf("%6i << %6i || %6.3lf ms/sim ||--||", evaluations_, newBarrier_,
        timer_.getTimeMS() / (evaluations_ - evaluationsLastTime_));
    evaluationsLastTime_ = evaluations_;
    
    iterator = populations_.begin();
    for (int i = 0; iterator != populations_.end() && i < 3; ++iterator, ++i)
      printf(" %4i %8.2f%c ||", iterator->getPoolSize(), iterator->getBest(),
          (iterator->getIsPaused()) ? '-' : '+');
    printf("\n");

    /*Robot robot;
     robot.initialize(&environment_);
     robot.position_[0] = 0.1;
     robot.position_[1] = -0.1;
     robot.speedVector_[0] = 0.0;
     robot.speedVector_[1] = 0.1;
     robot.head_ = 90.0;
     //robot.rotate(1.0, MAX_SPEEDSQ);
     for (int i = 0; i < 10000; ++i)
     {
     robot.accelerate(0.0, robot.speedVector_[0]*robot.speedVector_[0]
     + robot.speedVector_[1]*robot.speedVector_[1]);
     }
     printf("%f %f %f\n", robot.speedVector_[0], robot.speedVector_[1],
     robot.head_);*/

    timer_.start();
  }
}

void Plga::finish()
{
  bigTimer_.end();
  printf("\n==================================\nSummary:\n\n");
  printf("Total simulations made: %i\n", evaluations_);
  printf("Total time: %.1f s\n", bigTimer_.getTimeS());
  printf("Average ms / simulation: %.4f ms\n", bigTimer_.getTimeMS()
      / evaluations_);

  puts("\nIntergrity:\nAll of these 4 should be equal:");
  printf("\n%f\n", populations_.front().getBest());
  populations_.front().score(0, false);
  printf("%f\n", populations_.front().getBest());
  populations_.front().score(0, false);
  printf("%f\n", populations_.front().getBest());
  populations_.front().score(0, true);
  printf("%f\n", populations_.front().getBest());
}
