#include "SysHeaders.hpp"
#include "Engine.hpp"

extern int GliderAppMain(int argc,char* argv[]);

/*
const int padding = 64;

int seq = 0;
void* myalloc(size_t size)
{
  uint8_t* ptr = (uint8_t*)malloc(size+padding*2);
  memset(ptr, 0xaa, padding);
  memset(ptr + padding + size, 0xbb, padding);
  printf("[%d]alloc:%p\n", seq++, ptr+padding);
  return ptr + padding;
}

void* mycalloc(size_t count, size_t size)
{
  uint8_t* ptr = (uint8_t*)malloc(size*count+padding*2);
  memset(ptr, 0xaa, padding);
  memset(ptr+padding, 0, size*count);
  memset(ptr + padding + size*count, 0xbb, padding);
  printf("[%d]calloc:%p\n", seq++, ptr+padding);
  return ptr + padding;
}

void* myrealloc(void* vptr, size_t newsize)
{
  uint8_t* ptr=(uint8_t*)vptr;
  uint8_t* rv = (uint8_t*)realloc(ptr?ptr-padding:nullptr, newsize+padding*2);
  memset(rv+padding+newsize, 0xbb, padding);
  printf("[%d]realloc:%p->%p\n", seq++, ptr, rv+padding);
  return rv+padding;
}

void myfree(void* vptr)
{
  uint8_t* ptr=(uint8_t*)vptr;
  printf("[%d]free:%p\n", seq++, ptr);
  free(ptr-padding);
}
*/
extern "C"
int main(int argc,char* argv[])
{
  //SDL_SetMemoryFunctions(myalloc, mycalloc, myrealloc, myfree );
  glider::engine.init();
  int rc=GliderAppMain(argc,argv);
  SDL_Quit();
  return rc;
}
