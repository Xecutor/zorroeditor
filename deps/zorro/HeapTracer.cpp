#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <unistd.h>
#define _XOPEN_SOURCE 1
#include <ucontext.h>
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>
//#include <link.h>

#if defined(linux) || defined(__APPLE__)
#include <execinfo.h>
#else
#include <sys/frame.h>
#endif

#ifdef linux

static void BackTrace(void** dump)
{
  backtrace(dump,MAXTRACESIZE);
}
#else

#if defined(sparc) || defined(__sparc)
#define FRAME_PTR_REGISTER REG_SP
#ifdef __sparcv9
#define BIAS 2047
#else
#define BIAS 0
#endif
#endif

static const int MAXTRACESIZE=30;

struct ctxdata{
  void** dump;
  int cnt;
};

static int stackWalk(uintptr_t pc,int flags,void* data)
{
  ctxdata* d=(ctxdata*)data;
  if(d->cnt==MAXTRACESIZE)return -1;
  d->dump[d->cnt++]=(void*)pc;
  return 0;
}

static void BackTrace(void** dump)
{
  int counter=0;

#ifndef __APPLE__
  ucontext_t u;
  getcontext(&u);
#endif

#ifdef sparc
  frame* fp=(struct frame*)((long)(u.uc_mcontext.gregs[FRAME_PTR_REGISTER]) + BIAS);

  void* savpc;
  frame* savfp;

  int skip=2;

  while(
    ((unsigned long)fp)>0x1000 &&
    (savpc = ((void*)fp->fr_savpc)) &&
    counter < MAXTRACESIZE
  )
  {
    if(skip==0)
    {
      dump[counter]=savpc;
      ++counter;
    }else
    {
      skip--;
    }
    fp = (struct frame*)((long)(fp->fr_savfp) + BIAS);
  }
  if(counter!=MAXTRACESIZE)dump[counter]=0;
#else
#ifdef __APPLE__
  int cnt=backtrace(dump,MAXTRACESIZE);
  if(cnt<MAXTRACESIZE)dump[cnt]=0;
#else
  ctxdata d;
  d.dump=dump;
  d.cnt=0;
  walkcontext(&u,stackWalk,&d);
  if(d.cnt!=MAXTRACESIZE)dump[d.cnt]=0;
#endif
#endif
}
#endif

typedef void* (*malloc_func)(size_t);
typedef void (*free_func)(void*);
typedef void* (*realloc_func)(void*,size_t);

static malloc_func libc_malloc=0;
static free_func libc_free=0;
static realloc_func libc_realloc=0;

static void ht_init()
{
//  printf("htinit\n");
#ifdef __APPLE__
  void* h=dlopen("/usr/lib/libSystem.B.dylib",RTLD_NOW);
#else
  void* h=dlopen("/usr/lib/64/libc.so",RTLD_NOW);
#endif
  libc_malloc=(malloc_func)dlsym(h,"malloc");
  libc_free=(free_func)dlsym(h,"free");
  libc_realloc=(realloc_func)dlsym(h,"realloc");
}

const int btsize=(18+18+1)*MAXTRACESIZE+32;

#ifdef __APPLE__
#define BTSTR "image lookup -a %p\n"
#else
#define BTSTR "whereis -a %p\n"
#endif

#define printbt \
  void* dump[MAXTRACESIZE];\
  BackTrace(dump);\
  for(int i=0;dump[i] && i<MAXTRACESIZE;++i)\
  {\
    ptr+=sprintf(ptr,BTSTR,dump[i]);\
  }\
  write(1,buf,ptr-buf);

extern "C" void* malloc(size_t sz)
{
  if(!libc_malloc)ht_init();
  void* rv=libc_malloc(sz);
  char buf[2+16+2+20+1+btsize];
  char* ptr=buf;
  ptr+=sprintf(ptr,"M:%p[%lu]\n",rv,sz);
  printbt;
  return rv;
}

extern "C" void free(void* mem)
{
  char buf[2+16+1+btsize];
  char* ptr=buf;
  ptr+=sprintf(ptr,"F:%p\n",mem);
  printbt;

  libc_free(mem);
}

extern "C" void* realloc(void* mem,size_t sz)
{
  char buf[2+16+2+16+2+20+1+btsize];
  char* ptr=buf;
  void* rv=libc_realloc(mem,sz);
  ptr+=sprintf(ptr,"R:%p->%p[%lu]\n",mem,rv,sz);
  printbt;
  return rv;
}


void* operator new(size_t sz)
{
  return malloc(sz);
}

void operator delete(void* ptr)
{
  free(ptr);
}

void* operator new[](size_t sz)
{
  return malloc(sz);
}

void operator delete[](void* ptr)
{
  free(ptr);
}

