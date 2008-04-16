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

#define FIRE_COUNT 2

class Environment
{
  private:
    double* walls_;
    int wallCount_;
    double fires_[FIRE_COUNT][2];
  public:
    int getWallCount()
    {
      return wallCount_;
    }
    void initialise(const char*);
    void kill();
    double getCorner(int wallNumber, int corner)
    {
      return walls_[wallNumber*4+corner];
    }
    double getFire(int fire, int axis)
    {
      return fires_[fire][axis];
    }
};

void Environment::initialise(const char* fileName)
{
  FILE* fin;
  fin = fopen(fileName, "r");
  for (int i = 0; i < FIRE_COUNT; ++i) {
    fscanf(fin, "%lf %lf", &fires_[i][0], &fires_[i][1]);
  }
  fscanf(fin, "%i", &wallCount_);
  walls_ = new double[wallCount_ * 4];
  for (int i = 0; i < wallCount_; ++i)
  {
    fscanf(fin, "%lf %lf %lf %lf", &walls_[i*4+0], &walls_[i*4+1],
        &walls_[i*4+2], &walls_[i*4+3]);
  }
  fclose(fin);
}

void Environment::kill()
{
  delete [] walls_;
}
