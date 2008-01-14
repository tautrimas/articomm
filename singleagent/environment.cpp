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

class CEnvironment {
  private:
    double *wall;
    int walls;
  public:
    CEnvironment() {}
    ~CEnvironment() {}
    int wallcount() {return walls;}
    void initwalls();
    void killwalls();
    double getcorner(int wallnumber, int corner) {return wall[wallnumber*4+corner];}
};

void CEnvironment::initwalls()
{
  ifstream fin("walls.txt");
  fin >> walls;
  wall = new double[walls*4];
  for (int i=0; i<walls; i++){
    fin >> wall[i*4+0];
    fin >> wall[i*4+1];
    fin >> wall[i*4+2];
    fin >> wall[i*4+3];
  }
  fin.close();
}

void CEnvironment::killwalls()
{
  delete [] wall;
}
