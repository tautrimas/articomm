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

#include <cstdlib>
#include <time.h>

class CRandomization {
  public:
    CRandomization();
    ~CRandomization() {}
    double randdouble();
    double randdouble(double min, double max);
  private:
    time_t seconds;
};

CRandomization::CRandomization() {
  time(&seconds);
  srand((unsigned int) seconds);
}

double CRandomization::randdouble()
{
  return rand()/(double(RAND_MAX)+1);
}

//returns random double from min to max. Borrowed source code.
double CRandomization::randdouble(double min, double max)
{
  if (min>max)
  {
    return randdouble()*(min-max)+max;
  }
  else
  {
    return randdouble()*(max-min)+min;
  }
}
