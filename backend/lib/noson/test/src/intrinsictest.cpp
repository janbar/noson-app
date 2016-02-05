#if (defined(_WIN32) || defined(_WIN64))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#include <winsock2.h>
#include <Windows.h>
#include <time.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include "../../noson/src/private/os/threads/threadpool.h"
#include "../../noson/src/intrinsic.h"

#include <cstdio>
#include <stdlib.h>

NSROOT::IntrinsicCounter* g_counter = 0;

class WorkerInc : public NSROOT::OS::CWorker
{
  virtual void Process()
  {
    for (int i = 0; i < 5000000; i++)
    {
      g_counter->Increment();
    }
  }
};

class WorkerDec : public NSROOT::OS::CWorker
{
  virtual void Process()
  {
    for (int i = 0; i < 5000000; i++)
    {
      g_counter->Decrement();
    }
  }
};

int main(int argc, char** argv)
{
  int val = 0;
  if (argc > 1)
    val = atoi(argv[1]);
  g_counter = new NSROOT::IntrinsicCounter(val);
  NSROOT::OS::CThreadPool pool(20);
  pool.Suspend();
  for (int i = 0; i < 5; ++i)
  {
    pool.Enqueue(new WorkerInc());
    pool.Enqueue(new WorkerDec());
  }
  pool.SetKeepAlive(100);
  pool.Resume();
  unsigned ps;
  while ((ps = pool.Size()) > 0)
  {
    printf("Running (%2u): counter=%d\n", ps, g_counter->GetValue());
    usleep(10000);
  }
  printf("Completed   : counter=%d\n", g_counter->GetValue());
  delete g_counter;
  return EXIT_SUCCESS;
}