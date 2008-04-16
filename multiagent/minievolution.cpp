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

struct MiniGeneArray
{
    double* genes;
    double score;
};

struct MiniValueInterval
{
    double a, b;
};

bool operator<(const MiniGeneArray& a, const MiniGeneArray& b)
{
  return a.score > b.score;
}

class MiniEvolution
{
  public:
    MiniEvolution(int geneCount);
    MiniEvolution(int geneCount, int popSize);
    void initialise(int, int);
    ~MiniEvolution();
    void setValue(int n, double val)
    {
      for (int i = 0; i < popSize_; ++i)
      {
        genes_[i].genes[n] = val;
      }
    }
    double getValue(int val);
    void addScore(double score);
    void addScore(double score, int liquid);
    void setRequiredVolume(int count)
    {
      requiredVolume_ = count;
    }
    void setInterval(int gene, double a, double b)
    {
      intervals_[gene].a = a;
      intervals_[gene].b = b;
    }
  private:
    void mutateGenes(int parent, int child);
    void evolve();
    MiniGeneArray* genes_;
    MiniValueInterval* intervals_;

    int requiredVolume_;
    int geneCount_;
    int popSize_;

    int currentVolume_;
    int currentMember_;
};

MiniEvolution::MiniEvolution(int geneCount)
{
  initialise(geneCount, 3);
}

MiniEvolution::MiniEvolution(int geneCount, int popSize)
{
  initialise(geneCount, popSize);
}

void MiniEvolution::initialise(int geneCount, int popSize)
{
  popSize_ = popSize;
  geneCount_ = geneCount;
  genes_ = new MiniGeneArray[popSize_];
  for (int i = 0; i < popSize; ++i)
  {
    genes_[i].genes = new double[geneCount_];
  }
  intervals_ = new MiniValueInterval[geneCount_];
  for (int i = 0; i < geneCount_; ++i)
  {
    intervals_[i].a = -1.0e100;
    intervals_[i].b = 1.0e100;
  }
  currentMember_ = popSize_ - 1;
  currentVolume_ = 10000;
  requiredVolume_ = 5;
  for (int i = 0; i < popSize_; ++i)
  {
    genes_[i].score = 0.0;
  }
}

MiniEvolution::~MiniEvolution()
{
  for (int i = 0; i < popSize_; ++i)
  {
    delete [] genes_[i].genes;
  }
  delete [] genes_;
  delete [] intervals_;
}

double MiniEvolution::getValue(int val)
{
  return genes_[currentMember_].genes[val];
}

void MiniEvolution::addScore(double score)
{
  addScore(score, 1);
}

void MiniEvolution::addScore(double score, int liquid)
{
  genes_[currentMember_].score += score;

  if (currentVolume_ < requiredVolume_ - 1)
  {
    currentVolume_ += liquid;
  }
  else if (currentMember_ < popSize_ - 1)
  {
    genes_[currentMember_].score /= currentVolume_ + 1;
    currentVolume_ = 0;
    ++currentMember_;
  }
  else
  {
    genes_[currentMember_].score /= currentVolume_ + 1;
    currentMember_ = 0;
    currentVolume_ = 0;
    evolve();
  }
}

void MiniEvolution::mutateGenes(int parent, int child)
{
  int i = rand() % geneCount_;
  double x = R.randDouble(-1.4422, 0.9086);
  genes_[child].genes[i] = genes_[parent].genes[i] * ((-x * x * x) + 1.0);

  if (genes_[child].genes[i] > intervals_[i].b)
  {
    genes_[child].genes[i] = intervals_[i].b;
  }
  else if (genes_[child].genes[i] < intervals_[i].a)
  {
    genes_[child].genes[i] = intervals_[i].a;
  }
}

void MiniEvolution::evolve()
{
  std::sort(genes_, genes_ + popSize_);
  for (int i = 2; i < popSize_; ++i)
  {
    mutateGenes(i % 1, i);
    genes_[i].score *= 1.0e-20;
  }
  for (int i = 0; i < popSize_; ++i)
  {
    genes_[i].score = 0.0;
  }
}
