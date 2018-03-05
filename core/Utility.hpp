#pragma once

#include <vector>
#include <kst/Format.hpp>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

namespace glider{

struct Pos{
  float x,y;
  Pos():x(0.0f),y(0.0f)
  {
  }
  Pos(float argX,float argY):x(argX),y(argY)
  {
  }
  template <class T>
  Pos(const T& argOther):x((float)argOther.x),y((float)argOther.y){}
  bool isZero()const
  {
    return x==0.0f && y==0.0f;
  }
  Pos operator-()const
  {
    return Pos(-x,-y);
  }

  Pos& operator+=(const Pos& argPos)
  {
    x+=argPos.x;
    y+=argPos.y;
    return *this;
  }

  Pos& operator-=(const Pos& argPos)
  {
    x-=argPos.x;
    y-=argPos.y;
    return *this;
  }

  Pos& operator/=(float argVal)
  {
    x/=argVal;
    y/=argVal;
    return *this;
  }

  Pos& operator*=(float argVal)
  {
    x*=argVal;
    y*=argVal;
    return *this;
  }

  Pos& operator*=(const Pos& argVal)
  {
    x*=argVal.x;
    y*=argVal.y;
    return *this;
  }

  Pos& clampX(float minVal,float maxVal)
  {
    if(x<minVal)x=minVal;
    if(x>maxVal)x=maxVal;
    return *this;
  }

  Pos& clampY(float minVal,float maxVal)
  {
    if(y<minVal)y=minVal;
    if(y>maxVal)y=maxVal;
    return *this;
  }

  Pos& clamp(Pos minVal,Pos maxVal)
  {
    clampX(minVal.x,maxVal.x);
    clampY(minVal.y,maxVal.y);
    return *this;
  }


  friend Pos operator+(const Pos& lhs,const Pos& rhs)
  {
    return Pos(lhs.x+rhs.x,lhs.y+rhs.y);
  }
  friend Pos operator-(const Pos& lhs,const Pos& rhs)
  {
    return Pos(lhs.x-rhs.x,lhs.y-rhs.y);
  }
  Pos xOnly()const
  {
    return Pos(x,0.0f);
  }
  Pos yOnly()const
  {
    return Pos(0.0f,y);
  }
  friend Pos operator*(const Pos& pos,float val)
  {
    return Pos(pos.x*val,pos.y*val);
  }
  friend Pos operator*(const Pos& lhs,const Pos& rhs)
  {
    return Pos(lhs.x*rhs.x,lhs.y*rhs.y);
  }
  friend Pos operator/(const Pos& pos,float val)
  {
    return Pos(pos.x/val,pos.y/val);
  }
  float length()const
  {
    return sqrt(x*x+y*y);
  }
  void select();
  void selectTex();
  Pos& round()
  {
    x=roundf(x);
    y=roundf(y);
    return *this;
  }
  float sum()const
  {
    return fabsf(x)+fabsf(y);
  }
};

template <class T>
struct Posi{
  T x,y;
  Posi():x(0),y(0)
  {
  }
  Posi(T argX,T argY):x(argX),y(argY)
  {
  }
  template <class U>
  Posi(const U& argOther):x(argOther.x),y(argOther.y){}
  bool isZero()const
  {
    return x==0.0f && y==0.0f;
  }

  template <class U>
  bool operator==(const U& other)const
  {
    return x==other.x && y==other.y;
  }
  template <class U>
  bool operator!=(const U& other)const
  {
    return x!=other.x || y!=other.y;
  }
  friend Posi operator+(const Posi& lhs,const Posi& rhs)
  {
    return Posi(lhs.x+rhs.x,lhs.y+rhs.y);
  }
  friend Posi operator-(const Posi& lhs,const Posi& rhs)
  {
    return Posi(lhs.x-rhs.x,lhs.y-rhs.y);
  }
  Posi xOnly()
  {
    return Posi(x,0.0f);
  }
  Posi yOnly()
  {
    return Posi(0.0f,y);
  }
  friend Posi operator*(const Posi& pos,T val)
  {
    return Posi(pos.x*val,pos.y*val);
  }
  friend Posi operator/(const Posi& pos,T val)
  {
    return Pos(pos.x/val,pos.y/val);
  }
  bool operator<(const Posi& pos)const
  {
    return x<pos.x || (x==pos.x && y<pos.y);
  }
  void select();
  void selectTex();
};

template <>
void Posi<unsigned short>::select();

template <>
void Posi<unsigned int>::select();

template <>
void Posi<unsigned short>::selectTex();

template <>
void Posi<unsigned int>::selectTex();


struct Rect{
  Pos pos;
  Pos size;
  Rect(){}
  Rect(float x,float y,float w,float h):pos(x,y),size(w,h){}
  Rect(const Pos& argPos,const Pos& argSize):pos(argPos),size(argSize){}
  Pos tl()const
  {
    return pos;
  }
  Pos tr()const
  {
    return Pos(pos.x+size.x,pos.y);
  }
  Pos bl()const
  {
    return Pos(pos.x,pos.y+size.y);
  }
  Pos br()const
  {
    return pos+size;
  }
  bool isInside(const Pos& argPos)const
  {
    return argPos.x>pos.x && argPos.y>pos.y && argPos.x<pos.x+size.x && argPos.y<pos.y+size.y;
  }
  void pushQuad(std::vector<Pos>& v)const
  {
    v.push_back(tl());
    v.push_back(tr());
    v.push_back(br());
    v.push_back(bl());
  }
  void setQuad(std::vector<Pos>& v,size_t pos)const
  {
    v[pos]=tl();
    v[pos+1]=tr();
    v[pos+2]=br();
    v[pos+3]=bl();
  }
  friend Rect operator+(const Rect& rect,const Pos& shift)
  {
    return Rect(rect.pos+shift,rect.size);
  }
  friend Rect operator-(const Rect& rect,const Pos& shift)
  {
    return Rect(rect.pos-shift,rect.size);
  }
};

template <class T>
struct Recti{
  Posi<T> pos;
  Posi<T> size;
  Recti(){}
  Recti(const Posi<T>& argPos,const Posi<T>& argSize):pos(argPos),size(argSize){}
  Recti(T x,T y,T w,T h):pos(x,y),size(w,h){}
  bool isInside(const Posi<T>& argPos)const
  {
    return argPos.x>=pos.x && argPos.y>=pos.y && argPos.x<pos.x+size.x && argPos.y<pos.y+size.y;
  }
  bool onBorder(const Posi<T>& argPos)const
  {
    return argPos.x==pos.x || argPos.y==pos.y || argPos.x==pos.x+size.x-1 || argPos.y==pos.y+size.y-1;
  }
  Posi<T> tl()const
  {
    return pos;
  }
  Posi<T> tr()const
  {
    return Posi<T>(pos.x+size.x,pos.y);
  }
  Posi<T> bl()const
  {
    return Posi<T>(pos.x,pos.y+size.y);
  }
  Posi<T> br()const
  {
    return pos+size;
  }
  void pushQuad(std::vector<Posi<T> >& v)
  {
    v.push_back(tl());
    v.push_back(tr());
    v.push_back(br());
    v.push_back(bl());
  }
  struct Iterator{
    Posi<T> pos;
    Posi<T> end;
    int startx;
    Iterator(Posi<T> argPos, Posi<T> argSize)
    {
      pos=argPos;
      startx=pos.x;
      end=pos+argSize;
    }
    Posi<T> operator*()const
    {
      return pos;
    }
    Iterator& operator++()
    {
      ++pos.x;
      if(pos.x>=end.x) {
        ++pos.y;
        pos.x=startx;
      }
      return *this;
    }
    Iterator operator++(int)
    {
      Iterator rv = *this;
      ++pos.x;
      if(pos.x>=end.x) {
        ++pos.y;
        pos.x=startx;
      }
      return rv;
    }
    bool operator==(const Iterator& argOther)const
    {
      return pos==argOther.pos;
    }
    bool operator!=(const Iterator& argOther)const
    {
      return pos!=argOther.pos;
    }
  };
  Iterator begin()const
  {
    return {pos, size};
  }
  Iterator end()const
  {
    Posi<T> p=pos;
    p.y+=size.y;
    return {p,{0,0}};
  }
};

struct Grid{
  Grid(const Pos& argSize,int argHSize,int argVSize):size(argSize),
      hsize(argHSize),vsize(argVSize),cell(size.x/hsize,size.y/vsize)
  {
    hspace=0;
    vspace=0;
  }

  void setSpacing(int argHSpace,int argVSpace)
  {
    hspace=argHSpace;
    vspace=argVSpace;
  }

  Rect getCell(int x,int y)
  {
    return Rect(Pos(cell.x*x+hspace*x+hspace,cell.y*y+vspace*y+vspace),cell);
  }

  Pos size;
  int hsize;
  int vsize;
  int hspace;
  int vspace;
  Pos cell;

};

struct Color{
  float r,g,b,a;
  Color():r(1.0f),g(1.0f),b(1.0f),a(1.0f)
  {
  }
  Color(float argR,float argG,float argB,float argA=1.0):
    r(argR),g(argG),b(argB),a(argA)
  {
  }
  Color(unsigned int clr,bool useAlfa=false)
  {
    a=useAlfa?((clr>>24)&0xff)/255.0f:1.0f;
    r=((clr&0xff0000)>>16)/255.0f;
    g=((clr&0x00ff00)>>8)/255.0f;
    b=((clr&0x0000ff))/255.0f;
  }

  uint32_t packTo32()const
  {
    uint32_t rv=(uint32_t)(a*255);
    rv<<=8;
    rv|=(uint32_t)(r*255);
    rv<<=8;
    rv|=(uint32_t)(g*255);
    rv<<=8;
    rv|=(uint32_t)(b*255);
    return rv;
  }

  friend Color operator+(const Color& left,const Color& right)
  {
    return Color(left.r+right.r,left.g+right.g,left.b+right.b,1);
  }

  friend Color operator/(const Color& clr,float v)
  {
    return Color(clr.r/v,clr.g/v,clr.b/v,clr.a);
  }

  friend Color operator*(const Color& clr,float v)
  {
    return Color(clr.r*v,clr.g*v,clr.b*v,clr.a);
  }

  void tint(const Color& t)
  {
    r*=t.r;
    g*=t.g;
    b*=t.b;
  }


  Color& operator/=(float v)
  {
    r/=v;
    g/=v;
    b/=v;
    return *this;
  }

  Color& operator*=(float v)
  {
    r*=v;
    g*=v;
    b*=v;
    return *this;
  }


  Color& operator+=(const Color& clr)
  {
    r+=clr.r;
    g+=clr.g;
    b+=clr.b;
    a+=clr.a;
    return *this;
  }

  void toHSL(float& h, float& s, float& l)
  {
    float mn, mx;
    mn = r < g ? r : g;
    mn = mn  < b ? mn  : b;
    mx = r > g ? r : g;
    mx = mx  > b ? mx  : b;

    l = mx;
    float delta = mx - mn;
    if (delta < 0.00001f)
    {
        s = 0.0f;
        h = 0.0f; // undefined, maybe nan?
        return;
    }
    if( mx > 0.0f ) {
        s = (delta / mx);
    } else {
        s = 0.0f;
        h = 0.0f;
        return;
    }
    if( r >= mx )
        h = ( g - b ) / delta;
    else
    if( g >= mx )
        h = 2.0f + ( b - r ) / delta;
    else
        h = 4.0f + ( r - g ) / delta;

    h *= 60.0f;

    if( h < 0.0f ) {
        h += 360.0f;
    }
  }
  
  void fromHSL(float h, float s, float l)
  {
    float hh, p, q, t, ff;
    long  i;

    if(s <= 0.0f) {
        r = l;
        g = l;
        b = l;
        return;
    }
    hh = h;
    if(hh >= 360.0f) {
      hh = fmodf(hh, 360.0f);
    }
    hh /= 60.0f;
    i = (long)hh;
    ff = hh - i;
    p = l * (1.0f - s);
    q = l * (1.0f - (s * ff));
    t = l * (1.0f - (s * (1.0f - ff)));

    switch(i) {
    case 0:
        r = l;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = l;
        b = p;
        break;
    case 2:
        r = p;
        g = l;
        b = t;
        break;

    case 3:
        r = p;
        g = q;
        b = l;
        break;
    case 4:
        r = t;
        g = p;
        b = l;
        break;
    case 5:
    default:
        r = l;
        g = p;
        b = q;
        break;
    }
  }

  void changeLightness(float mul)
  {
    float h,s,l;
    toHSL(h,s,l);
    l*=mul;
    if(l>1.0f)
    {
      l=1.0f;
    }
    fromHSL(h,s,l);
  }

  void changeSaturation(float mul)
  {
    float h,s,l;
    toHSL(h,s,l);
    s*=mul;
    if(s>=1.0f)
    {
      s=1.0f;
    }
    fromHSL(h,s,l);
  }

  Color& clamp()
  {
    if(r>1.0f)r=1.0f;
    if(r<0.0f)r=0.0f;
    if(g>1.0f)g=1.0f;
    if(g<0.0f)g=0.0f;
    if(b>1.0f)b=1.0f;
    if(b<0.0f)b=0.0f;
    if(a>1.0f)a=1.0f;
    if(a<0.0f)a=0.0f;
    return *this;
  }

  void select();
  static Color red;
  static Color green;
  static Color blue;
  static Color white;
  static Color black;
  static Color gray;
  static Color yellow;
  static Color cyan;
};

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

inline void customformat(kst::FormatBuffer& buf,const Pos& pos,int w,int p)
{
  char charBuf[128];
  int len=snprintf(charBuf, sizeof(charBuf), "%f,%f",pos.x,pos.y);
  buf.Append(charBuf,len);
}

}
