#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#ifdef _WIN32
#ifndef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif
#include <stdio.h>

#ifdef _WIN32
class Timer{
public:
  Timer()
  {
    start.QuadPart=0;
    QueryPerformanceFrequency(&freq);
  }
  void Start()
  {
    QueryPerformanceCounter(&start);
  }
  void Finish()
  {
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    start.QuadPart=(end.QuadPart-start.QuadPart);
  }
  __int64 Get()
  {
    return start.QuadPart*1000/freq.QuadPart;
  }
  __int64 GetMcs()
  {
    return start.QuadPart*1000000/freq.QuadPart;
  }

protected:
  LARGE_INTEGER start,freq;
};
#else
class Timer{
public:
  void Start()
  {
    gettimeofday(&_start,0);
  }
  void Finish()
  {
    gettimeofday(&_end,0);
  }
  long long Get()
  {
    long long rv=_end.tv_sec-_start.tv_sec;
    long long usec=_end.tv_usec-_start.tv_usec;
    return rv*1000+usec/1000;
  }
  long long GetMcs()
  {
    long long rv=_end.tv_sec-_start.tv_sec;
    long long usec=_end.tv_usec-_start.tv_usec;
    return rv*1000000+usec;
  }
protected:
  timeval _start;
  timeval _end;
};
#endif

class TimeThis{
public:
  TimeThis(const char* argMsg,int argCount):msg(argMsg),count(argCount)
  {
    t.Start();
  }
  operator bool(){return false;}
  ~TimeThis()
  {
    t.Finish();
    int ms=(int)t.Get();
    printf("%s:time=%dms,speed=%lf/sec\n",msg,ms,double(count*1000.0/ms));
  }
protected:
  const char* msg;
  int count;
  Timer t;
};

#define TIMETHIS(msg,n) if(TimeThis tt=TimeThis(msg,n));else for(int i=0;i<n;i++)

#endif
