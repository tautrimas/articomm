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

class Environment
{
  private:
    double* wall_;
    int walls_;
  public:
    Environment()
    {
    }
    ~Environment()
    {
    }
    int wallCount()
    {
      return walls_;
    }
    void initializeWalls();
    void killWalls();
    double getCorner(int wallNumber, int corner)
    {
      return wall_[wallNumber*4+corner];
    }
};

void Environment::initializeWalls()
{
  FILE* fin;
  fin = fopen("walls.txt", "r");
  fscanf(fin, "%i", &walls_);
  wall_ = new double[walls_*4];
  for (int i=0; i<walls_; i++)
  {
    fscanf(fin, "%lf %lf %lf %lf", &wall_[i*4+0], &wall_[i*4+1], &wall_[i*4+2],
        &wall_[i*4+3]);
  }
  fclose(fin);
}

void Environment::killWalls()
{
  delete [] wall_;
}
