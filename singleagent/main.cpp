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

CRandomization R;

#include <iostream>
#include <fstream>

#include "evolution.cpp"

int main()
{
  CEvolution evolution(dydis);
  do
  {
    //begin evolution
    do
    {
      evolution.step();
    } while (!(evolution.enough() ||
               //(evolution.getbest() < 9000.0 && evolution.getgeneration()*dydis > 10000) ||
               (evolution.getbest() < 13000.0 && evolution.getgeneration()*dydis > 200000) ||
                   (evolution.getbest() < 20000.0 && evolution.getgeneration()*dydis > 1200000)) || evolution.getcontin());
    if (!evolution.enough())
    {
      evolution.restart(dydis);
    }
  } while (!(evolution.enough()));
  evolution.finish();

  return 0;
}
