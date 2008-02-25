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

// g++ main.cpp -o main -O2 && chmod u+x main && time ./main
// g++ main.cpp -o main -g && chmod u+x main && ddd main

#include "header.cpp"
#include "random.cpp"

Randomization R;

#include <stdio.h>
#include <stdlib.h>

#include "plga.cpp"
#include "threads.cpp"

struct ThreadArgument
{
  Plga* plga;
  bool stop;
};

void* evolveRobots(void* arg)
{
  ThreadArgument* argument = (ThreadArgument*) arg;
  while (argument->stop != true)
    argument->plga->step();
  pthread_exit(0);
}

int main()
{
  /*
  CEvolution evolution(dydis);
  do
  {
    //begin evolution
    do
    {
      evolution.step();
    } while (!(evolution.enough() ||
               //(evolution.getbest() < 9000.0 && evolution.getgeneration()*dydis > 10000) ||
               (evolution.getbest() < 13000.0 && evolution.getgeneration()*dydis > 50000) ||
                   (evolution.getbest() < 20000.0 && evolution.getgeneration()*dydis > 300000)));
    if (!evolution.enough())
    {
      evolution.restart(dydis);
    }
  } while (!(evolution.enough()));
  evolution.finish();
  */
  ThreadArgument* argument = new ThreadArgument;
  argument->plga = new Plga(POPULATION_SIZE, 5, 300);
  argument->stop = false;
  Thread thread;
  thread.run(evolveRobots, argument);
  /*Plga plga(POPULATION_SIZE, 3, 20);
  plga.step();
  plga.finish();*/
  getc(stdin);
  argument->stop = true;
  thread.join();
  argument->plga->finish();
  
  delete argument->plga;
  delete argument;

  return 0;
}
