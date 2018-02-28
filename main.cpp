#define _DLGS_H
#include <stdio.h>
#include "Engine.hpp"
#include "ui/UIConfig.hpp"
#include "ui/UIRoot.hpp"
#include "ui/Splitter.hpp"
#include <kst/Logger.hpp>
#include "GLState.hpp"
#include "ResourcesManager.hpp"
#include "Console.hpp"
#include "Project.hpp"
#include "FileList.hpp"
#include <clocale>
#ifdef HEAP_DEBUG
#include <execinfo.h>
#endif

using namespace glider;
using namespace glider::ui;
using namespace zorro;

/*
#include "/Users/skv/smsc/src/util/leaktracing/heaptracer/HeapTracer.cpp"
#include <new>
void* operator new(size_t sz)throw(std::bad_alloc)
{
  void* rv=malloc(sz);
//  printf("new:%p[%lu]\n",rv,sz);
  return rv;
}

void operator delete(void* vptr)throw()
{
//  printf("delete:%p\n",vptr);
  free(vptr);
}

void* operator new[](size_t sz)throw(std::bad_alloc)
{
  void* rv=malloc(sz);
//  printf("new:%p[%lu]\n",rv,sz);
  return rv;
}

void operator delete[](void* vptr)throw()
{
//  printf("delete:%p\n",vptr);
  free(vptr);
}
*/


#ifdef HEAP_DEBUG
FILE* heapLog=fopen("heaplog.txt","w+");

const int preAlloc=32;
const int postAlloc=32;
const int preAllocPattern=0xAA;
const int allocPattern=0x00;
const int postAllocPattern=0xBB;

const int extraBytes=preAlloc+postAlloc+sizeof(size_t)+sizeof(void*);

unsigned char* llFirst=0;
//unsigned char* llLast=0;

void checkAll()
{
  if(llFirst)
  {
    unsigned char* ptr=llFirst;
    int cnt=0;
    while(ptr)
    {
      size_t sz=*(size_t*)ptr;
      ptr+=sizeof(size_t);
      unsigned char* next=*(unsigned char**)ptr;
      ptr+=sizeof(void*);
      for(int i=0;i<preAlloc;++i)
      {
        if(ptr[i]!=preAllocPattern)abort();
      }
      ptr+=preAlloc;
      for(int i=0;i<(int)sz;++i)
      {
        if(ptr[i]!=0xDD)abort();
      }
      ptr+=sz;
      for(int i=0;i<postAlloc;++i)
      {
        if(ptr[i]!=postAllocPattern)abort();
      }
      ptr=next;
      //if(++cnt==200)break;
    }
  }
}

void *myNew(size_t sz)
{
  unsigned char* ptr=(unsigned char*)malloc(sz+extraBytes);
  *((size_t*)ptr)=sz;
  ptr+=sizeof(sz);
  *((void**)ptr)=0;
  ptr+=sizeof(void*);
  for(int i=0;i<preAlloc;++i)ptr[i]=preAllocPattern;
  ptr+=preAlloc;
  void* rv=ptr;
  for(int i=0;i<(int)sz;++i)ptr[i]=allocPattern;
  ptr+=sz;
  for(int i=0;i<postAlloc;++i)ptr[i]=postAllocPattern;
  void* bt[20];
  int acnt=backtrace(bt,20);

  fprintf(heapLog,"new:%p,%d:",rv,int(sz));
  for(int i=0;i<acnt;++i)fprintf(heapLog,"%s%p",i==0?"":",",bt[i]);
  fprintf(heapLog,"\n");
  //fflush(heapLog);

  //checkAll();
  return rv;
}

void myDelete(void* vptr)
{
  fprintf(heapLog,"delete:%p\n",vptr);fflush(heapLog);
  unsigned char* ptr=(unsigned char*)vptr;
  ptr-=preAlloc+sizeof(size_t)+sizeof(void*);
  size_t sz=*((size_t*)ptr);
  ptr+=sizeof(size_t);
  ptr+=sizeof(void*);
  for(int i=0;i<preAlloc;++i)
  {
    if(ptr[i]!=preAllocPattern)abort();
  }
  ptr+=preAlloc;
  for(int i=0;i<(int)sz;++i)
  {
    ptr[i]=0xDD;
  }
  ptr+=sz;
  for(int i=0;i<postAlloc;++i)
  {
    if(ptr[i]!=postAllocPattern)abort();
  }
  ptr-=sz+preAlloc+sizeof(size_t)+sizeof(void*);
  //free(ptr);
  if(llFirst)
  {
    void** lptr=(void**)(ptr+sizeof(size_t));
    *lptr=llFirst;
    llFirst=ptr;
  }else
  {
    llFirst=ptr;
  }
  void* bt[20];
  int acnt=backtrace(bt,20);

  fprintf(heapLog,"delete:%p,%d:",vptr,int(sz));
  for(int i=0;i<acnt;++i)fprintf(heapLog,"%s%p",i==0?"":",",bt[i]);
  fprintf(heapLog,"\n");

  //checkAll();
}

void* operator new(size_t sz)throw(std::bad_alloc)
{
  return myNew(sz);
}

void operator delete(void* vptr)throw()
{
  myDelete(vptr);
}

void* operator new[](size_t sz)throw(std::bad_alloc)
{
  return myNew(sz);
}

void operator delete[](void* vptr)throw()
{
  myDelete(vptr);
}
#endif

int GliderAppMain(int argc,char* argv[])
{
  std::setlocale(LC_CTYPE,"");
  try{
#ifdef DEBUG
  dpFile=fopen("zorro.log","wb+");
#endif
    kst::Logger::Init("ze.log");
    kst::Logger* mlog=kst::Logger::getLogger("main");
    //engine.setVSync(false);
    engine.enableResizable();
    engine.selectResolution(1600,900,false);
    engine.setResolution();
    engine.setDefaultFont(manager.getFont("../data/FSEX300.ttf",16));
    uiConfig.init();
    engine.assignHandler(root);
    engine.setFpsLimit(60);
    LOGINFO(mlog,"vendor=%s, renderer=%s",state.getVendor(),state.getRenderer());


    Splitter* s1 = new Splitter(Splitter::soVertical, 200);
    s1->setSize(root->getSize());
    s1->setName("splitter1");
    s1->setResizePolicy(Splitter::rpKeepFirst);
    Splitter* s2 = new Splitter(Splitter::soHorizontal, root->getSize().y-200);
    s2->setName("splitter2");
    s2->setResizePolicy(Splitter::rpKeepSecond);

    root->addObject(s1);

    Console* console=new Console();
    Project prj(console);
    s1->setFirst(prj.getFileList());
    s1->setSecond(s2);

    int flWidth=prj.getFileList()->getSize().x;

    s2->setSecond(console);


    if(argc==2)
    {
      prj.load(argv[1]);
    }else
    {
      prj.addFile("unnamed.zs");
      prj.openOrSwitchFile("unnamed.zs");
    }
    prj.getFileList()->setFocus();

    //prj.getIndexer().importApi("zrlg.zapi");
    /*const char* files[]={
        "/Users/skv/Documents/cpp/zrlg/build/game/ani.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/colors.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/enemy.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/game.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/intro.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/items.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/keys.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/levelgen.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/main.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/player.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/suiteabilities.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/terrain.zs",
        "/Users/skv/Documents/cpp/zrlg/build/game/utility.zs"
    };
    for(auto f:files)
    {
      prj.addFile(f);
    }
    prj.openOrSwitchFile("/Users/skv/Documents/cpp/zrlg/build/game/game.zs");*/

    //prj.addFile("test.zs");
    //prj.addFile("test2.zs");
    //prj.openOrSwitchFile("test.zs");

    engine.loop(root);
    uiConfig.shutdown();

  }catch(std::exception& e)
  {
    printf("Exception:%s\n",e.what());
  }

#ifdef HEAP_DEBUG
  checkAll();
#endif

  return 0;
}
