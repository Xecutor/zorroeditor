#ifndef __GLIDER_VERTEXBUFFER_HPP__
#define __GLIDER_VERTEXBUFFER_HPP__

#include <vector>
#include "Utility.hpp"
#include "Drawable.hpp"

namespace glider{

enum Primitive{
  pPoints,pLines,pLinesStrip,pLinesLoop,pTriangles,pTriangleStrip,pTriangleFan,
  pQuads,pQuadStrip,pPolygon
};

struct VxVector:public std::vector<Pos>{
  void push4(const Pos& pos)
  {
    insert(end(),4,pos);
  }
};
struct ClrVector:public std::vector<Color>{
  void push4(const Color& clr)
  {
    insert(end(),4,clr);
  }
  void set4(const Color& clr,size_t idx)
  {
    (*this)[idx]=clr;
    (*this)[idx+1]=clr;
    (*this)[idx+2]=clr;
    (*this)[idx+3]=clr;
  }
};

enum UpdateFlag{
  ufVertices=1,
  ufTexture=2,
  ufColors=4,
  ufAll=ufVertices|ufTexture|ufColors
};

class VertexBuffer:public Drawable{
public:
  VertexBuffer(Primitive argPrimitive=pQuads):tenabled(false),cenabled(false),
    vbo(0),vbosize(0),size(0),autoSize(true),needIdent(false),primitive(argPrimitive)
  {
  }
  virtual ~VertexBuffer();
  VxVector& getVBuf()
  {
    return vbuf;
  }
  VxVector& getTBuf()
  {
    tenabled=true;
    return tbuf;
  }
  ClrVector& getCBuf()
  {
    cenabled=true;
    return cbuf;
  }
  const ClrVector& getCBuf()const
  {
    return cbuf;
  }
  void draw();
  void update(int updateFlags=ufAll,int from=0,int to=-1);
  void setSize(int argSize)
  {
    size=argSize;
    autoSize=false;
  }
  void loadIdentity(bool argValue)
  {
    needIdent=argValue;
  }
protected:
  VxVector vbuf,tbuf;
  ClrVector cbuf;
  bool tenabled,cenabled;
  uint vbo;
  int vbosize;
  int size;
  bool autoSize;
  bool needIdent;
  Primitive primitive;
  int vbufSize(int cnt=-1)
  {
    return (cnt==-1?vbuf.size():cnt)*sizeof(vbuf[0]);
  }
  int tbufSize(int cnt=-1)
  {
    return (cnt==-1?tbuf.size():cnt)*sizeof(tbuf[0]);
  }

  int cbufSize(int cnt=-1)
  {
    return (cnt==-1?cbuf.size():cnt)*sizeof(cbuf[0]);
  }

};

}

#endif
