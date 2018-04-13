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
#include <stdlib.h>
#include "VertexBuffer.hpp"
#include "GLState.hpp"
#include "Scissors.hpp"
#include <kst/Logger.hpp>
#include "VertexBuffer.hpp"
#include <set>
#include <vector>
#include <SDL2/SDL.h>
#include "kst/File.hpp"

#include "cfov.hpp"

using namespace glider;

const int CELL_SIZE=15;

struct Cell{
  TileInfo ti;
  bool vis;
};

typedef std::vector<Cell> Row;
typedef std::vector<Row> Matrix;

/*const char* testmap[]={
"########################################",
"#......................................#",
"#......................................#",
"#........................#########.....#",
"#......########..........#.......#.....#",
"#......#......############.......#.....#",
"#......#.........................|.....#",
"#......#......###########........|.....#",
"#......###--###.........#........#.....#",
"#.......................###.######.....#",
"#.........................#.#..........#",
"#......................####.####.......#",
"#.......#.#.#.#.#......................#",
"#........#.#.#.#.......#########.......#",
"#.......#.#.#.#.#......................#",
"#........#.#.#.#.......................#",
"#...........................#..........#",
"#.......#...................#..........#",
"#...........................###........#",
"#....#.................................#",
"#.............#........................#",
"#....................#.................#",
"#..#......#.........#..................#",
"#............###.####..................#",
"#.......#...........#.........##.......#",
"#............###.####.........##.......#",
"#......#.......#.#.....................#",
"#..............###.....................#",
"#......................................#",
"#......................................#",
"#......................................#",
"#.........*****........##--##..........#",
"#....**.......*........#....#..........#",
"#.............*........|....|..........#",
"#.............*........|....|..........#",
"#.............*........#....#..........#",
"#......................##..##..........#",
"#......................................#",
"#......................................#",
"########################################",
};*/




int px;
int py;
int sightRadius=15;
VertexBuffer* vb;
Line* lsa;
Line* lea;
Text* info;

void drawMap(Matrix& m)
{
  int H=m.size();
  int W=m[0].size();
  Color clr;
  ClrVector& c=vb->getCBuf();
  for(int y=0;y<H;++y)
  {
    for(int x=0;x<W;++x)
    {
      if(m[y][x].ti.block)clr=Color::white;else clr=Color::blue;
      if(x==px && y==py)clr=Color::red;
      if(m[y][x].ti.tempBlock)
      {
        clr=Color::green;
      }
      if(m[y][x].ti.delayBlock)
      {
        clr=Color::gray;
      }
      if(!m[y][x].vis)
      {
        clr=clr/2;
      }
      for(int i=0;i<4;++i)
      {
        c[(x+y*W)*4+i]=clr;
      }
    }
  }
  vb->update();
}

namespace kst{
void customformat(kst::FormatBuffer& buf,double v,int w,int p)
{
  char buf2[64];
  size_t len=sprintf(buf2,"%*.*lf",w,p,v);
  buf.Append(buf2,len);
}
}


CircularFov<Posi<int> > cf;

struct MapAdapter{
  Matrix& m;
  MapAdapter(Matrix& argM):m(argM){}
  TileInfo getTile(int x,int y,int r)
  {
    TileInfo rv=m[y][x].ti;
    if(rv.tempBlock)
    {
      rv.tempRange=r-1;
    }
    if(rv.delayBlock)
    {
      rv.delayRange=r+3;
    }
    return rv;
  }
  void setVisible(int x,int y,int r)
  {
    m[y][x].vis=true;
  }
  bool isInside(int x,int y)
  {
    return x>=0 && x<getWidth() && y>=0 && y<getHeight();
  }
  int getWidth()
  {
    return m[0].size();
  }
  int getHeight()
  {
    return m.size();
  }
  void clearVis()
  {
    for(size_t y=0;y<m.size();++y)
    {
      for(size_t x=0;x<m[y].size();++x)
      {
        m[y][x].vis=false;
      }
    }
  }
};


class MyEventHandler:public EventHandler{
public:
  MyEventHandler(Matrix& argM):m(argM)
  {
    viewAt.x=-1;
    viewAt.y=-1;
    dfov=false;
    minTransRate=0.5;
    viewAngle=AngleRange::SCALED_PI/3;
    dr.sa=AngleRange::addNorm(0,viewAngle);
    dr.ea=AngleRange::subNorm(0,viewAngle);
  }
  virtual ~MyEventHandler(){}
  void onActiveChange(bool active)
  {
  }
  void onMouseEvent(MouseEvent& argEvent)
  {
    if(argEvent.eventType==metButtonPress && argEvent.eventButton==1)
    {
      int x=argEvent.x/CELL_SIZE;
      int y=argEvent.y/CELL_SIZE;
      if(x<0 || x>=(int)m[0].size() || y<0 || y>=(int)m.size())return;
      viewAt.x=x;
      viewAt.y=y;
      updateViewAt();
    }
    if(argEvent.eventType==metButtonPress && argEvent.eventButton==3)
    {
      viewAt.x=-1;viewAt.y=-1;
      lsa->setSource(Pos(0,0));
      lsa->setDestination(Pos(10,0));
      lea->setSource(Pos(0,0));
      lea->setDestination(Pos(0,10));
    }
    bool updateDr=false;
    if(argEvent.eventType==metButtonPress && argEvent.eventButton==4)
    {
      if(viewAngle<AngleRange::SCALED_PI-AngleRange::SCALED_PI/18)viewAngle+=AngleRange::SCALED_PI/90;
      updateDr=true;
    }
    if(argEvent.eventType==metButtonPress && argEvent.eventButton==5)
    {
      if(viewAngle>AngleRange::SCALED_PI/18)viewAngle-=AngleRange::SCALED_PI/90;
      updateDr=true;
    }
    if(argEvent.eventButton==metMove)
    {
      updateDr=true;
    }
    if(dfov && updateDr)
    {
      int x=argEvent.x/CELL_SIZE;
      int y=argEvent.y/CELL_SIZE;
      if(x!=px || y!=py)
      {
        int dx=x-px;
        int dy=y-py;

        int a=AngleRange::getAngle(dx,dy);
        dr.sa=AngleRange::addNorm(a,viewAngle);
        dr.ea=AngleRange::subNorm(a,viewAngle);
        updateMap();
      }
    }
  }
  void updateViewAt()
  {
    int x=viewAt.x;
    int y=viewAt.y;
    if(x==-1 || y==-1)return;
    //printf("select %d,%d\n",x,y);
    //char buf[256];
    //sprintf(buf,"x=%d,y=%d,r=%d,sa=%.3lf,ea=%.3lf\nsac=%d,eac=%d\n",x,y,m[y][x].r,m[y][x].fc.r.saScaled(),m[y][x].fc.r.eaScaled(),m[y][x].fc.sac,m[y][x].fc.eac);
    //std::string txt=buf;
    //txt+=m[y][x].log;
    //if(!m[y][x].vis)
    //{
      //Posi<int> bp=m[y][x].;
      //sprintf(buf+len,"\nbsa=%.3lf,bea=%.3lf",m[y][x].bfc.sa/1000000.0,m[y][x].bfc.ea/1000000.0);
      /*
      m[bp.y][bp.x].mark=true;
      m[lastMark.y][lastMark.x].mark=false;
      lastMark=bp;*/
    //}
    //info->setText(txt.c_str());
    /*
    Pos src=Pos(px*CELL_SIZE+CELL_SIZE/2.0,py*CELL_SIZE+CELL_SIZE/2.0);
    lsa->setSource(src);
    lea->setSource(src);
    Pos p;
    int mr=(sightRadius+1)*CELL_SIZE;
    double sa=m[y][x].fc.r.sa*M_PI/AngleRange::SCALED_PI;
    p.x=src.x+mr*cos(sa);
    p.y=src.y+mr*sin(sa);
    lsa->setDestination(p);
    double ea=m[y][x].fc.r.ea*M_PI/AngleRange::SCALED_PI;
    p.x=src.x+mr*cos(ea);
    p.y=src.y+mr*sin(ea);
    lea->setDestination(p);*/
  }
  void onKeyboardEvent(KeyboardEvent& argEvent)
  {
    if(argEvent.keySym==keyboard::GK_ESCAPE)
    {
      engine.exitApp();
    }
    if(argEvent.eventType==ketPress)
    {
      int ox=px;
      int oy=py;
      if(argEvent.keySym==keyboard::GK_UP || argEvent.keySym==keyboard::GK_W)py--;
      if(argEvent.keySym==keyboard::GK_DOWN || argEvent.keySym==keyboard::GK_S)py++;
      if(argEvent.keySym==keyboard::GK_LEFT || argEvent.keySym==keyboard::GK_A)px--;
      if(argEvent.keySym==keyboard::GK_RIGHT || argEvent.keySym==keyboard::GK_D)px++;
      if(argEvent.keySym==keyboard::GK_KP_PLUS){sightRadius++;ox=-1;}
      if(argEvent.keySym==keyboard::GK_KP_MINUS && sightRadius>2){sightRadius--;ox=-1;}
      if(argEvent.keySym==keyboard::GK_X)
      {
        dfov=!dfov;
        updateMap();
      }
      if(argEvent.keySym==keyboard::GK_P)
      {
        if(minTransRate>0)minTransRate-=0.05f;
        if(minTransRate<=0.01f)minTransRate=0.01f;
        cf.setTransVisPrc(minTransRate,minTransRate/2.0f);
        updateMap();
      }
      if(argEvent.keySym==keyboard::GK_O)
      {
        if(minTransRate<=0.05f)minTransRate=0.05f;
        else if(minTransRate<1.0f)minTransRate+=0.05f;
        cf.setTransVisPrc(minTransRate,minTransRate/2);
        updateMap();
      }
      if(px<0 || px>=(int)m[0].size() || py<0 || py>=(int)m.size() || m[py][px].ti.block)
      {
        px=ox;
        py=oy;
      }
      if(ox!=px || oy!=py)
      {
        updateMap();
        updateViewAt();
      }
      if(argEvent.keySym==keyboard::GK_T)
      {
        uint32_t start=SDL_GetTicks();
        //cf.debug=false;
        MapAdapter ma(m);
        for(int i=0;i<1000;++i)
        {
          ma.clearVis();
          cf.generateFov(ma,Posi<int>(px,py),sightRadius);
        }
        //cf.debug=true;
        uint32_t duration=SDL_GetTicks()-start;
        info->setText(FORMAT("time=%dms",duration));
      }
    }
  }
  void updateMap()
  {
    MapAdapter ma(m);
    ma.clearVis();
    if(dfov)
    {
      cf.setPermBlock(dr,1);
    }else
    {
      cf.clearPermBlock();
    }
    cf.generateFov(ma,Posi<int>(px,py),sightRadius);
    drawMap(m);
  }
  void onResize(){};
  void onQuit()
  {
    engine.exitApp();
  }

  void onFrameUpdate(int mcsec)
  {
  }
  Matrix& m;
  Posi<int> viewAt;
  bool dfov;
  AngleRange dr;
  int viewAngle;
  float minTransRate;
};


int GliderAppMain(int argc,char* argv[])
{
  kst::Logger::Init("sample.log");
  engine.setVSync(false);
  engine.selectResolution(1024,768,false);
  //engine.selectResolution(800,480,false);
  engine.setResolution();
  engine.setFpsLimit(60);


  //const int W=strlen(testmap[0]);
  //const int H=sizeof(testmap)/sizeof(testmap[0]);
  kst::File f;
  kst::Logger* lg=kst::Logger::getLogger("init");
  try{
    f.ROpen("testmap.txt");
  }catch(std::exception& e)
  {
    LOGERROR(lg,"exception:%s",e.what());
    return 0;
  }
  Matrix m;
  //m.resize(H,Row(W,Cell()));

  //for(int y=0;y<H;++y)
  size_t y=0;
  std::string line;
  while(f.ReadLine(line))
  {
    m.push_back(Row(line.size(),Cell()));
    if(line.size()!=m[0].size())
    {
      LOGERROR(lg,"invalid line[%d] length=%d in testmap.txt, expected %d",y,line.size(),m[0].size());
      return 0;
    }
    for(size_t x=0;x<line.size();++x)
    {
      Cell& c=m[y][x];
      c.ti.delayBlock=line[x]=='|' || line[x]=='-';
      c.ti.tempBlock=line[x]=='*' || c.ti.delayBlock;
      c.ti.block=line[x]=='#' || c.ti.tempBlock || c.ti.delayBlock;
      c.ti.tempRange=2;
      c.ti.delayRange=2;
      c.vis=true;
      c.ti.corners[0]=100;
      c.ti.corners[1]=100;
      c.ti.corners[2]=100;
      c.ti.corners[3]=100;
    }
    ++y;
  }
  int W=m[0].size();
  int H=m.size();
  int cornAngle=60;
  for(int y=0;y<H;++y)
  {
    for(int x=0;x<W;++x)
    {
      if(m[y][x].ti.block)
      {
        if(y>0 && x>0 && !m[y-1][x].ti.block && !m[y][x-1].ti.block)
        {
          m[y][x].ti.corners[3]=cornAngle;
        }
        if(y>0 && x<W-1 && !m[y-1][x].ti.block && !m[y][x+1].ti.block)
        {
          m[y][x].ti.corners[2]=cornAngle;
        }
        if(y<H-1 && x<W-1 && !m[y+1][x].ti.block && !m[y][x+1].ti.block)
        {
          m[y][x].ti.corners[1]=cornAngle;
        }
        if(y<H-1 && x>0 && !m[y+1][x].ti.block && !m[y][x-1].ti.block)
        {
          m[y][x].ti.corners[0]=cornAngle;
        }
      }
    }
  }
  px=W/2;
  py=H/2;
  while(m[py][px].ti.block)px++;

  MyEventHandler meh(m);
  engine.assignHandler(&meh);


  engine.enableKeyboardRepeat();
  Scene sc;
  FontRef fnt1=manager.getFont("FSEX300.ttf",16);
  Text* txt=new Text(fnt1,"hello");
  txt->setPosition(Pos(10,10));
  vb=new VertexBuffer;
  lsa=new Line(Pos(0,0),Pos(10,0));
  lea=new Line(Pos(0,0),Pos(0,10));
  info=new Text(fnt1,
      "wasd/cursor to move\nnumpad+/numpad- to change sight radius\n"
      "o to decrease permissiveness, p to increase it\n"
      "x to toggle directional fov\nlook around with mouse with directional fov\n"
      "wheel up/down to change view angle\n"
      "t to run benchmark (1000 cycles of fov)");
  info->setPosition(Pos((float)W*CELL_SIZE,0.0f));
  VxVector& v=vb->getVBuf();
  ClrVector& c=vb->getCBuf();
  for(int y=0;y<H;++y)
  {
    for(int x=0;x<W;++x)
    {
      //glider::Rectangle* r=new glider::Rectangle(Rect(x*15,y*15,10,10),Color(1,0.1+x/20+y/20,0.1));
      //sc.addObject(r);
      Rect((float)x*CELL_SIZE+1,(float)y*CELL_SIZE+1,(float)CELL_SIZE-2,(float)CELL_SIZE-2).pushQuad(v);
      c.push4(Color::gray);
    }
  }
  vb->update();
  cf.prepare(sightRadius);
  MapAdapter ma(m);
  ma.clearVis();
  cf.generateFov(ma,Posi<int>(px,py),sightRadius);
  drawMap(m);
  sc.addObject(vb);
  //sc.addObject(lsa);
  //sc.addObject(lea);
  sc.addObject(info);
  //sc.addObject(txt);
  engine.loop(&sc);

//  for(CircularFov::Ring::iterator it=cf.fov[2].begin(),end=cf.fov[2].end();it!=end;++it)
//  {
//    FovCell& fc=*it;
//    printf("%d,%d:%.3lf-%.3lf\n",fc.p.x,fc.p.y,fc.sa/1000000.0,fc.ea/1000000.0);
//  }
//  for(CircularFov::Ring::iterator it=cf.fov[3].begin(),end=cf.fov[3].end();it!=end;++it)
//  {
//    FovCell& fc=*it;
//    printf("%d,%d:%.3lf-%.3lf\n",fc.p.x,fc.p.y,fc.sa/1000000.0,fc.ea/1000000.0);
//  }

  return 0;
}
