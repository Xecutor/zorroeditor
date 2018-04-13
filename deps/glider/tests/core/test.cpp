#include "Rectangle.hpp"
#include "Engine.hpp"
#include "Line.hpp"
#include "Image.hpp"
#ifndef _WIN32
#include <unistd.h>
#endif
#include "Scene.hpp"
#include "Text.hpp"
#include "Timer.hpp"
#include <time.h>
#include "ResourcesManager.hpp"
#include <string>
#include "VertexBuffer.hpp"
#include "GLState.hpp"
#include "Scissors.hpp"
#include "kst/Logger.hpp"

using namespace glider;

class MyVB:public VertexBuffer{
public:
  void draw()
  {
    state.translate(Pos(20,400));
    VertexBuffer::draw();
  }
};

Pos goff;

class MyEventHandler:public EventHandler{
public:
  MyEventHandler()
  {
    std::string s0("0123456789");
    fps=0;
    last=0;
    for(int i=0;i<7;i++)s+=s0;
    a=0;
    scale=1;
    img=new Image;
    fpsTxt=new Text;
  }
  void onActiveChange(bool active)
  {
    /*
    printf("active:%s\n",active?"true":"false");
    if(!active)
    {
      engine.setFpsLimit(0);
    }else
    {
      engine.setFpsLimit(-1);
      engine.setVSync(true);
    }
    */

  }
  void onMouseEvent(MouseEvent& argEvent){}
  void onKeyboardEvent(KeyboardEvent& argEvent)
  {
    if(argEvent.eventType==ketRelease)
    {
      return;
    }
    if(argEvent.keySym==keyboard::GK_ESCAPE)
    {
      engine.exitApp();
    }
    if(argEvent.keySym==keyboard::GK_UP)
    {
      goff.y-=1;
      state.popOffset();
      state.pushOffset(goff);
    }
    if(argEvent.keySym==keyboard::GK_DOWN)
    {
      goff.y+=1;
      state.popOffset();
      state.pushOffset(goff);
    }
    if(argEvent.keySym==keyboard::GK_LEFT)
    {
      goff.x-=1;
      state.popOffset();
      state.pushOffset(goff);
    }
    if(argEvent.keySym==keyboard::GK_RIGHT)
    {
      goff.x+=1;
      state.popOffset();
      state.pushOffset(goff);
    }
    if(argEvent.keySym==keyboard::GK_D)
    {
      a+=1;
      img->setAngle(a);
    }
    if(argEvent.keySym==keyboard::GK_A)
    {
      a-=1;
      img->setAngle(a);
    }
    if(argEvent.keySym==keyboard::GK_W)
    {
      scale+=0.01f;
      img->setScale(scale);
    }
    if(argEvent.keySym==keyboard::GK_S)
    {
      scale-=0.01f;
      img->setScale(scale);
    }

  }
  void onResize(){};
  void onQuit()
  {
    engine.exitApp();
  }

  void onFrameUpdate(int mcsec)
  {
    /*s+=s[0];
    s.erase(0,1);
    txt[0]->setText(s.c_str());
    fps++;
    time_t now=time(0);
    if(now!=last)
    {
      char fpsBuf[32];
      sprintf(fpsBuf,"%d",fps);
      printf("fps:%s\n",fpsBuf);
      fpsTxt->setText(fpsBuf);
      fps=0;
      //      printf("draw time=%lld\n",t.Get());
      last=now;
    }*/
  }
  float a;
  float scale;
  time_t last;
  int fps;
  Text* txt[100];
  FontRef fnt1,fnt2;
  TextRef fpsTxt;
  std::string s;
  ImageRef img;
};

class ScissorsTest:public Drawable{
public:
  void init()
  {
    fnt=manager.getFont("FSEX300.ttf",18);
    txt.assignFont(fnt.get());
    txt.setText("A this is sample");
    txt.setPosition(Pos(600,600));
    scis.setRect(Rect(600,600,60,60));
  }
  void draw()
  {
    scis.draw(&txt);
    //txt.draw();
  }
protected:
  Text txt;
  FontRef fnt;
  Scissors scis;
};

int GliderAppMain(int argc,char* argv[])
{
  kst::Logger::Init("test.log");
  engine.setVSync(true);
  engine.selectResolution(1024,768,false);
  //engine.selectResolution(800,480,false);
  engine.setResolution();

  MyEventHandler meh;
  engine.assignHandler(&meh);
  engine.setDefaultFont(manager.getFont("FreeMono.ttf"/*"FSEX300.ttf"*/,16));

  //engine.setFpsLimit(30);
  try{
  Scene sc;
  const char* str="AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz123456780!@#$%^&*()_+[]{;'\\:\"|},./<>?";
  TextRef t=new Text(engine.getDefaultFont(),str);
  FontRef glow=manager.getFont(engine.getDefaultFont()->getFileName().c_str(),engine.getDefaultFont()->getSize(),engine.getDefaultFont()->getFlags()|ffGlow);
  TextRef t2=new Text(glow,str);
  sc.addObject(t2.get());
  sc.addObject(t.get());

  glider::Rectangle* r=new glider::Rectangle(Rect(0.0f,0.0f,1024.0f,768.0f),Color(1.0f,0.1f,0.1f));

  sc.addObject(r);

  Rect rr(Pos(0.0f,0.0f),Pos((float)engine.getWidth()-1,(float)engine.getHeight()-1));
  Line* l1=new Line(rr.tl(),rr.tr());
  sc.addObject(l1);
  Line* l2=new Line(rr.tr(),rr.br());
  sc.addObject(l2);
  Line* l3=new Line(rr.br(),rr.bl());
  sc.addObject(l3);
  Line* l4=new Line(rr.bl(),rr.tl());
  sc.addObject(l4);

  meh.img->load("potion_bubbly.png");
  meh.img->setPosition(Pos(20,20));
  sc.addObject(meh.img.get());

  VertexBuffer* vbl=new VertexBuffer(pLinesStrip);
  vbl->loadIdentity(true);
  Color lclr(0.5,1.0,0.5);
  for(int i=0;i<10;i++)
  {
    vbl->getVBuf().push_back(Pos(700.5f,50.5f+i*10));
    vbl->getVBuf().push_back(Pos(750.5f,50.5f+i*10));
    vbl->getCBuf().push_back(lclr);
    vbl->getCBuf().push_back(lclr);
  }
  vbl->setSize(10*2);
  vbl->update();
  sc.addObject(vbl);


  //float angle=0;
  //float scale=1.0;

  meh.fnt1=manager.getFont("FSEX300.ttf",16,0);
  meh.fpsTxt->assignFont(meh.fnt1.get());
  //fnt1.loadTTF("FreeMono.ttf",18,true);


  for(int i=0;i<2;i++)
  {
    meh.txt[i]=new Text(meh.fnt1.get(),meh.s.c_str(),false, 100);
    meh.txt[i]->setPosition(Pos(5.0f,200.0f+i*meh.fnt1->getLineSkip()));
    sc.addObject(meh.txt[i]);
  }

  printf("font height=%d,line skip=%d, ascent=%d\n",meh.fnt1->getHeight(),meh.fnt1->getLineSkip(),meh.fnt1->getAscent());

  //char fpsBuf[64];


  meh.fpsTxt->setPosition(Pos(engine.getWidth()-30.0f,0.0f));

  sc.addObject(meh.fpsTxt.get());

  Image* img2=new Image;
  img2->create(64,64);
  glider::Rectangle* rt2=new glider::Rectangle(Rect(Pos(0,0),Pos(64,64)),Color(1,1,0));
  img2->renderHere(rt2);
  img2->renderHere(meh.img.get());
  img2->setPosition(Pos(400,150));
  sc.addObject(img2);

  Image* img3=new Image;
  img3->load("Untitled.png");
  img3->setPosition(Pos(350,500));
  sc.addObject(img3);


  Text* prov=new Text(meh.fnt1.get(),state.getRenderer());
  prov->setPosition(Pos(50,50));
  Text* vend=new Text(meh.fnt1.get(),state.getVendor());
  vend->setPosition(Pos(50.0f,50.0f+meh.fnt1->getHeight()));
  sc.addObject(prov);
  sc.addObject(vend);

  Text* rus=new Text(meh.fnt1.get(),"BLAH! это тест по русски\nsdf ^rkasnd^< kfajsnd ^bfkasjnd^< ^gfkas^< dnfkasdnf kasdnkf ^359WEIRD^< aksdnjf kasjdn fkasj ndfkasjn dfkasndfka",false, 200,true);
  rus->setPosition(Pos(50.0f,100.0f));
  glider::Rectangle* rr2=new glider::Rectangle(Rect(Pos(50.0f,100.0f),Pos((float)rus->getWidth(),(float)rus->getHeight())),Color(0.2f,0.2f,0.2f));
  sc.addObject(rr2);
  sc.addObject(rus);

  MyVB* vb=new MyVB;
  Grid gr(Pos(100,100),10,10);

  for(int x=0;x<10;x++)
  {
    for(int y=0;y<10;y++)
    {
      gr.getCell(x,y).pushQuad(vb->getVBuf());
      Color clr(0.5f+0.5f*x/10,0.5f+0.5f*y/10,0.5f,1.0f);
      vb->getCBuf().push_back(clr);
      vb->getCBuf().push_back(clr);
      vb->getCBuf().push_back(clr);
      vb->getCBuf().push_back(clr);
    }
  }
  vb->update();
  //vb->setSize(400);

  sc.addObject(vb);
  ScissorsTest* st=new ScissorsTest;
  st->init();
  sc.addObject(st);

//  Scissors sciss;
//  sciss.assignObj(&sc);
//  sciss.setRect(Rect(50,50,800,800));*/
  engine.enableKeyboardRepeat();

  state.pushOffset(Pos(0,0));
  engine.loop(&sc);
  engine.setDefaultFont(0);

  }catch(std::exception& e)
  {
    fprintf(stderr, "Exception: %s\n", e.what());
  }

  return 0;
}
