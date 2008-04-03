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

/*struct CompareMiniGenes
 {
 bool operator()(MiniGeneArray& a, MiniGeneArray& b)
 {
 return a.score > b.score;
 }
 };*/

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
    void setRequiredScores(int count)
    {
      requiredScores_ = count;
    }
  private:
    void mutateGenes(int parent, int child);
    void evolve();
    MiniGeneArray* genes_;

    int requiredScores_;
    int geneCount_;
    int popSize_;

    int currentScoring_;
    int currentMember_;
};

MiniEvolution::MiniEvolution(int geneCount)
{
  initialise(geneCount, 6);
}

MiniEvolution::MiniEvolution(int geneCount, int popSize)
{
  initialise(geneCount, popSize);
}

void MiniEvolution::initialise(int geneCount, int popSize)
{
  genes_ = new MiniGeneArray[popSize];
  for (int i = 0; i < popSize; ++i)
  {
    genes_[i].genes = new double[geneCount];
  }
  popSize_ = popSize;
  geneCount_ = geneCount;
  currentMember_ = popSize - 1;
  currentScoring_ = 5;
  requiredScores_ = 5;
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
}

double MiniEvolution::getValue(int val)
{
  return genes_[currentMember_].genes[val];
}

void MiniEvolution::addScore(double score)
{
  genes_[currentMember_].score += score;

  if (currentScoring_ < requiredScores_ - 1)
  {
    ++currentScoring_;
  }
  else if (currentMember_ < popSize_ - 1)
  {
    currentScoring_ = 0;
    ++currentMember_;
  }
  else
  {
    currentMember_ = 0;
    currentScoring_ = 0;
    evolve();
  }
}

void MiniEvolution::mutateGenes(int parent, int child)
{
  for (int i = 0; i < geneCount_; ++i)
  {
    double x = R.randDouble(-1, 0.87);
    genes_[child].genes[i] = genes_[parent].genes[i] * ((-x * x * x * x * x) + 1.0);
  }
}

void MiniEvolution::evolve()
{
  std::sort(genes_, genes_ + popSize_);
  for (int i = 2; i < popSize_; ++i)
  {
    mutateGenes(i % 2, i);
  }
  for (int i = 0; i < popSize_; ++i)
  {
    genes_[i].score = 0.0;
  }
}
