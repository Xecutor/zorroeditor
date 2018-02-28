#ifndef __GLIDER_LINE_HPP__
#define __GLIDER_LINE_HPP__

#include "Drawable.hpp"
#include "Utility.hpp"

namespace glider{

class Line:public Drawable{
public:
  Line():width(1.0f)
  {
  }
  Line(const Pos& argSrc,const Pos& argDst):src(argSrc),dst(argDst)
  {
    width=1.0f;
    srcColor=Color(1,1,1,1);
    dstColor=Color(1,1,1,1);
  }
  void setColors(const Color& argSrcColor,const Color& argDstColor)
  {
    srcColor=argSrcColor;
    dstColor=argDstColor;
  }
  void setWidth(float argWidth)
  {
    width=argWidth;
  }
  void setSource(const Pos& argSrc)
  {
    src=argSrc;
  }
  void setDestination(const Pos& argDst)
  {
    dst=argDst;
  }
  Pos getSource()const
  {
    return src;
  }
  Pos getDestination()const
  {
    return dst;
  }
  void draw();
protected:
  Pos src,dst;
  Color srcColor,dstColor;
  float width;
};

}

#endif
