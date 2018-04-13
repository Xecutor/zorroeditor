/*
 * cfov.hpp
 *
 *  Created on: 10.05.2013
 *      Author: Xecutor
 *      Version: 1.0
 */

#ifndef CFOV_HPP_
#define CFOV_HPP_

#include <math.h>
#include <inttypes.h>

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2    1.57079632679489661923
#endif


struct AngleRange{
  int sa,ea;
  bool closed;
  static const int SCALED_PI_2 =90000000;
  static const int SCALED_PI  =180000000;
  static const int SCALED_PIx2=360000000;
  AngleRange(int argSa=0,int argEa=0):sa(argSa),ea(argEa),closed(false){}
  void setSa(int argSa)
  {
    if(!closed && ((sa<ea && argSa>ea)||(sa>ea && argSa<sa && argSa>ea)))closed=true;
    sa=argSa;
  }
  void setEa(int argEa)
  {
    if(!closed && ((sa<ea && sa>argEa)||(sa>ea && argEa>ea && argEa<sa)))closed=true;
    ea=argEa;
  }
  bool operator<(const AngleRange& other)const
  {
    return ea<other.ea;
  }
  double saScaled()
  {
    return sa/1000000.0;
  }
  double eaScaled()
  {
    return ea/1000000.0;
  }
  int middle()
  {
    if(sa<ea)return (sa+ea)/2;
    return (sa+ea+SCALED_PIx2)%SCALED_PIx2;
  }
  void shrinkCorners(int saPrc,int eaPrc)
  {
    if(sa>ea)
    {
      ea+=AngleRange::SCALED_PIx2;
    }
    uint64_t mid=middle();
    uint64_t val=mid-sa;
    val*=saPrc;
    val/=100;
    sa=(mid-val)%AngleRange::SCALED_PIx2;
    val=ea-mid;
    val*=eaPrc;
    val/=100;
    ea=(mid+val)%AngleRange::SCALED_PIx2;
  }
  int diff()
  {
    if(sa<ea)return ea-sa;
    return (ea+SCALED_PIx2-sa)%SCALED_PIx2;
  }
  bool inRange(int a)
  {
    if(closed)return false;
    if(sa<ea)return a>=sa && a<=ea;
    return a>=sa || a<=ea;
  }
  void mirror45()
  {
    sa=(SCALED_PI*2+SCALED_PI/2-sa)%(SCALED_PI*2);
    ea=(SCALED_PI*2+SCALED_PI/2-ea)%(SCALED_PI*2);
    std::swap(sa,ea);
  }
  void mirror90()
  {
    sa=(SCALED_PI*3-sa)%(SCALED_PI*2);
    ea=(SCALED_PI*3-ea)%(SCALED_PI*2);
    std::swap(sa,ea);
  }
  void mirror180()
  {
    sa=(SCALED_PI*4-sa)%(SCALED_PI*2);
    ea=(SCALED_PI*4-ea)%(SCALED_PI*2);
    std::swap(sa,ea);
  }
  static int getAngle(double x,double y)
  {
    double q=0;
    if(x<0 && y>=0){q=M_PI;}
    else if(x<0 && y<0){q=M_PI;}
    else if(x>=0 && y<0){q=2*M_PI;}

    return (int)((atan(y/x)+q)*(SCALED_PI_2)/M_PI_2);
  }
  static int addNorm(int ang,int inc)
  {
    return (ang+inc)%SCALED_PIx2;
  }
  static int subNorm(int ang,int inc)
  {
    return (ang-inc+SCALED_PIx2)%SCALED_PIx2;
  }
};

template <class P>
struct FovCell{
  AngleRange r;
  P p;
  uint8_t sac,eac;
  uint8_t o1,o2;
  bool tempBlock;
  bool delayBlock;
  int trange,drange;
  bool operator<(const FovCell& other)const
  {
    return r<other.r;
  }
  void swapp()
  {
    std::swap(p.x,p.y);
  }
  void swapc()
  {
    std::swap(sac,eac);
  }
};


struct TileInfo{
  bool block;
  bool tempBlock;
  bool delayBlock;
  uint8_t corners[4];
  int tempRange;
  int delayRange;
  TileInfo():block(false),tempBlock(false),delayBlock(false),tempRange(0),delayRange(0)
  {
    corners[0]=100;
    corners[1]=100;
    corners[2]=100;
    corners[3]=100;
  }
};


template <class P>
class CircularFov{
public:

  CircularFov()
  {
    minTransRate=0.5f;
    midTransRate=0.2f;
    havePermBlock=false;
  }

  void setTransVisPrc(float minRate,float midRate)
  {
    minTransRate=minRate;
    midTransRate=midRate;
  }

  void prepare(int maxRadius)
  {
    if(fov.empty())fov.push_back(Ring());
    for(int r=fov.size();r<=maxRadius;++r)
    {
      fov.push_back(Ring());
      Ring& ring=fov.back();
      //double rr=r*r;//(r+0.5)*(r+0.5);
      for(int y=0;y<=r;++y)
      {
        for(int x=y;x<=r;++x)
        {
          if(x==0 && y==0)continue;
          double dx=x;//-0.5;
          double dy=y;//-0.5;
          if(floor(sqrt(dx*dx+dy*dy))<=r)
          {
            P p(x,y);
            if(processed.find(p)!=processed.end())continue;
            processed.insert(p);
            int sa,ea,sac,eac,o1,o2;
            /*if(x==0)
            {
              sa=getAngle(0.5,y-0.5,0);
              ea=getAngle(-0.5,y-0.5,M_PI_2);
            }else
            {*/
            if(y==0)
            {
              sa=AngleRange::getAngle(x-0.5,-0.5);
              ea=AngleRange::getAngle(x-0.5,0.5)-1;
              sac=3;
              eac=0;
              o1=0;
              o2=7;
            }else if(x==y)
            {
              sa=AngleRange::getAngle(x+0.5,y-0.5);
              ea=AngleRange::getAngle(x-0.5,y+0.5);
              sac=2;
              eac=0;
              o1=0;
              o2=1;
            }else
            {
              sa=AngleRange::getAngle(x+0.5,y-0.5);
              ea=AngleRange::getAngle(x-0.5,y+0.5);
              sac=2;
              eac=0;
              o1=o2=0;
            }
            //}
            ring.push_back(FovCell<P>());
            FovCell<P>& fc=ring.back();
            fc.r.sa=sa+5;
            fc.r.ea=ea-5;
            fc.sac=sac;
            fc.eac=eac;
            fc.o1=o1;
            fc.o2=o2;
            fc.p=p;
          }
        }
      }
      size_t end=ring.size();
      for(size_t i=0;i<end;++i)
      {
        FovCell<P> fc=ring[i];
        if(fc.p.x==fc.p.y)continue;
        fc.swapp();
        fc.r.mirror45();
        fc.sac=fc.sac==0?2:fc.sac==2?0:fc.sac;
        fc.eac=fc.eac==0?2:fc.eac==2?0:fc.eac;
        fc.swapc();
        fc.o1=1;
        fc.o2=fc.o2==7?2:1;
        ring.push_back(fc);
      }
      end=ring.size();
      int ct[4]={1,0,3,2};
      for(size_t i=0;i<end;++i)
      {
        FovCell<P> fc=ring[i];
        if(fc.p.x==0)continue;
        fc.p.x=-fc.p.x;
        fc.r.mirror90();
        fc.sac=ct[fc.sac];
        fc.eac=ct[fc.eac];
        fc.swapc();
        fc.o1=fc.o1==0?3:2;
        fc.o2=fc.o2==7?4:fc.o2==0?3:2;
        ring.push_back(fc);
      }
      end=ring.size();
      int ct2[4]={3,2,1,0};
      for(size_t i=0;i<end;++i)
      {
        FovCell<P> fc=ring[i];
        if(fc.p.y==0)continue;
        fc.p.y=-fc.p.y;
        fc.r.mirror180();
        fc.sac=ct2[fc.sac];
        fc.eac=ct2[fc.eac];
        fc.swapc();
        fc.o1=7-fc.o1;
        fc.o2=7-fc.o2;
        ring.push_back(fc);
      }
      //std::sort(ring.begin(),ring.end());
    }
  }

  template<class MapAdapter>
  void generateFov(MapAdapter& m,P p,int maxRadius)
  {
    if(maxRadius<=0)return;
    if((int)fov.size()<maxRadius+1)
    {
      prepare(maxRadius);
    }
    for(int i=0;i<8;++i)
    {
      b[i].clear();
      if(havePermBlock)
      {
        b[i].push_back(permBlock);
      }
    }

    m.setVisible(p.x,p.y,0);

    for(int ri=1;ri<=maxRadius;++ri)
    {
      Ring& ring=fov[ri];
      for(typename Ring::iterator it=ring.begin(),end=ring.end();it!=end;++it)
      {
        FovCell<P> fc=*it;
        int x=p.x+fc.p.x;
        int y=p.y+fc.p.y;
        if(!m.isInside(x,y))continue;
        TileInfo c=m.getTile(x,y,ri);
        fc.delayBlock=c.delayBlock;
        fc.tempBlock=c.tempBlock;
        fc.trange=ri+c.tempRange;
        fc.drange=ri+c.delayRange;
        AngleRange r=fc.r;
        bool totalBlock=false;
        bool saBlock;
        bool usedTempBlock=false;
        bool tempInEffect=false;
        bool vis=false;
        int oa[2]={fc.o1,fc.o2};
        int ocnt=oa[0]==oa[1]?1:2;
        for(int k=0;k<ocnt;++k)
        {
          int o=oa[k];

          for(size_t i=0;i<b[o].size();++i)
          {
            FovCell<P> bc=b[o][i];
            tempInEffect=false;
            if(bc.delayBlock || bc.tempBlock)
            {
              tempInEffect=bc.tempBlock && !bc.delayBlock;
              if(bc.delayBlock && bc.tempBlock)
              {
                if(ri>bc.trange && ri<=bc.drange)continue;
                tempInEffect=ri<=bc.drange;
              }else if(bc.delayBlock && ri<=bc.drange)continue;
              else if(bc.tempBlock && ri>bc.trange)continue;
            }
            saBlock=false;
            if(bc.r.inRange(r.sa))
            {
              r.setSa(bc.r.ea);
              saBlock=true;
              usedTempBlock=usedTempBlock || tempInEffect;
            }
            if(bc.r.inRange(r.ea))
            {
              r.setEa(bc.r.sa);
              usedTempBlock=usedTempBlock || tempInEffect;
              if(saBlock)
              {
                totalBlock=true;
                break;
              }
            }
            if(r.closed)break;
            if(!c.block && r.inRange(bc.r.sa) && r.inRange(bc.r.ea))
            {
              totalBlock=true;
              usedTempBlock=usedTempBlock || tempInEffect;
              break;
            }
          }
          if(r.closed || totalBlock)break;
        }
        if(c.block)
        {
          if(usedTempBlock || (!totalBlock && !r.closed))
          {
            m.setVisible(x,y,ri);
            vis=true;
          }
        }else
        {
          if(!totalBlock)
          {
            int r1=r.diff();
            int r2=fc.r.diff();
            if(r1>=r2*minTransRate || (r1>=r2*midTransRate && r.inRange(fc.r.middle())))
            {
              m.setVisible(x,y,ri);
              vis=true;
            }
          }
        }

        if(vis && c.block)
        {
          if(c.corners[fc.sac]!=100 || c.corners[fc.eac]!=100)
          {
            fc.r.shrinkCorners(c.corners[fc.sac],c.corners[fc.eac]);
          }
          b[fc.o1].push_back(fc);
          if(fc.o1!=fc.o2)
          {
            b[fc.o2].push_back(fc);
          }
        }
      }
    }
  }

  void setPermBlock(AngleRange r,int delay=0)
  {
    permBlock.delayBlock=delay>0;
    permBlock.drange=delay;
    permBlock.tempBlock=false;
    permBlock.r=r;
    permBlock.p=P(0,0);
    havePermBlock=true;
  }
  void clearPermBlock()
  {
    havePermBlock=false;
  }

protected:
  typedef std::vector<FovCell<P> > Ring;
  typedef std::vector<Ring> Circle;

  float minTransRate;
  float midTransRate;

  Circle fov;
  std::set<P> processed;
  std::vector<FovCell<P> > b[8];
  bool havePermBlock;
  FovCell<P> permBlock;
};



#endif /* CFOV_HPP_ */
