#include <sys/time.h>

class Benchmarker
{
  public:
    Benchmarker();
    ~Benchmarker();
    void start();
    void end();
    double getTimeS();
    double getTimeMS();
  private:
    timeval timeStarted_;
    timeval timeEnded_;
    bool isStart, isEnd;
};

Benchmarker::Benchmarker()
{
  isStart = false;
  isEnd = false;
}

Benchmarker::~Benchmarker()
{
}

INL void Benchmarker::start()
{
  gettimeofday(&timeStarted_, NULL);
  isStart = true;
  isEnd = false;
}

INL void Benchmarker::end()
{
  gettimeofday(&timeEnded_, NULL);
  isEnd = true;
}

double Benchmarker::getTimeS()
{
  if (isStart && isEnd)
  {
    return (double)(timeEnded_.tv_sec - timeStarted_.tv_sec)
        + (double)(timeEnded_.tv_usec - timeStarted_.tv_usec) * 1.e-6;
  }
  else
  {
    return -1.0;
  }
}

double Benchmarker::getTimeMS()
{
  if (isStart && isEnd)
  {
    return (double)(timeEnded_.tv_sec - timeStarted_.tv_sec) * 1.e3
        + (double)(timeEnded_.tv_usec - timeStarted_.tv_usec) * 1.e-3;
  }
  else
  {
    return -1.0;
  }
}
