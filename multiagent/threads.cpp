#include <pthread.h>

class Thread
{
    public:
        Thread()
        {
        }
        ~Thread()
        {
        }
        int run(void* (funct)(void* arg), void* arg);
        void join();
        void pause()
        {
        }
        void resume()
        {
        }
        void kill();
    private:
        pthread_t threadId;
};

int Thread::run(void* (funct)(void* arg), void* arg)
{
    return pthread_create(&threadId, 0, funct, arg);
}

void Thread::join()
{
    pthread_join(threadId, 0);
}

void Thread::kill()
{
    pthread_cancel(threadId);
}
