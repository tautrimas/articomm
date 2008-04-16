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

#define INL inline

#include "random.cpp"

Randomization R;

#include <stdio.h>
#include <stdlib.h>

#include "threads.cpp"
#include "purplega.cpp"

struct ThreadArgument
{
    PurpleGa* purpleGa;
    bool stop;
};

void* evolveRobots(void* arg)
{
  ThreadArgument* argument = (ThreadArgument*) arg;
  while (argument->stop != true)
    argument->purpleGa->step();
  pthread_exit(0);
}

int main(int argc, char* argv[])
{
  int threadsForPool = 6;
  if (argc > 1)
  {
    sscanf(argv[1], "%i", &threadsForPool);
  }
  ThreadArgument* argument = new ThreadArgument;
  argument->purpleGa = new PurpleGa(15, 2, 6, threadsForPool);
  argument->stop = false;
  Thread thread;
  thread.run(evolveRobots, argument);

  /*for (int i = 0; i < 10; ++i)
   {
   Plga plga(6, 3, 6, threadsForPool);
   for (; plga.getEvaluations() <= 12000;)
   {
   plga.step();
   }
   plga.finish();
   }*/

  getc(stdin);
  argument->stop = true;
  thread.join();
  argument->purpleGa->finish();
  delete argument->purpleGa;
  delete argument;

  puts("");
  puts("Evolution complete! Go ahead - run: make animation && ./animateagent");
  puts("");

  return 0;
}
