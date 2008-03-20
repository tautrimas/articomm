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
#include "plga.cpp"

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

int main(int argc, char* argv[])
{
  int threadsForPool = 6;
  if (argc > 1)
  {
    sscanf(argv[1], "%i", &threadsForPool);
  }
  ThreadArgument* argument = new ThreadArgument;
  argument->plga = new Plga(6, 3, 6, threadsForPool);
  argument->stop = false;
  Thread thread;
  thread.run(evolveRobots, argument);

  /*Plga plga(600, 3, 20000000);
   for (int i = 0; i < 1000; ++i)
   {
   plga.step();
   }
   plga.finish();*/
  getc(stdin);
  argument->stop = true;
  thread.join();
  argument->plga->finish();
  delete argument->plga;
  delete argument;
  
  puts("");
  puts("Evolution complete! Go ahead - run: make animation && ./animateagent");
  puts("");

  return 0;
}
