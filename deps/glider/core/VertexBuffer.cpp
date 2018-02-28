#include "VertexBuffer.hpp"
#include "SysHeaders.hpp"
#include "GLState.hpp"

namespace glider{

VertexBuffer::~VertexBuffer()
{
  if(vbo)
  {
    glDeleteBuffers(1,&vbo);
  }
}


void VertexBuffer::draw()
{
  if(!vbo || vbosize==0)
  {
    return;
  }
  if(tenabled)
  {
    state.enableTexture();
  }else
  {
    state.disableTexture();
  }
  if(needIdent)
  {
    state.loadIdentity();
  }
  GLCHK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  GLCHK(glEnableClientState(GL_VERTEX_ARRAY));
  if(tenabled)
  {
    GLCHK(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
  }
  if(cenabled)
  {
    GLCHK(glEnableClientState(GL_COLOR_ARRAY));
  }
  GLCHK(glVertexPointer(2, GL_FLOAT, 0, 0));
  if(tenabled)
  {
    GLCHK(glTexCoordPointer(2,GL_FLOAT,0,(void*)(vbufSize())));
  }
  if(cenabled)
  {
    GLCHK(glColorPointer(4,GL_FLOAT,0,(void*)(vbufSize()+(tenabled?tbufSize():0))));
  }
  int p=GL_QUADS;
  switch(primitive)
  {
    case pPoints:p=GL_POINTS;break;
    case pLines:p=GL_LINES;break;
    case pLinesStrip:p=GL_LINE_STRIP;break;
    case pLinesLoop:p=GL_LINE_LOOP;break;
    case pTriangles:p=GL_TRIANGLES;break;
    case pTriangleStrip:p=GL_TRIANGLE_STRIP;break;
    case pTriangleFan:p=GL_TRIANGLE_FAN;break;
    case pQuads:p=GL_QUADS;break;
    case pQuadStrip:p=GL_QUAD_STRIP;break;
    case pPolygon:p=GL_POLYGON;break;
  }
  GLCHK(glDrawArrays(p,0,size));
  if(cenabled)
  {
    GLCHK(glDisableClientState(GL_COLOR_ARRAY));
  }
  if(tenabled)
  {
    GLCHK(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
  }
  GLCHK(glDisableClientState(GL_VERTEX_ARRAY));
  GLCHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

}


void VertexBuffer::update(int updateFlags,int from,int to)
{
  int newvbosize=vbufSize()+(tenabled?tbufSize():0)+(cenabled?cbufSize():0);

  int mx=vbuf.size();
  if(tenabled && (int)tbuf.size()<mx)
  {
    mx=tbuf.size();
  }
  if(cenabled && (int)cbuf.size()<mx)
  {
    mx=cbuf.size();
  }
  if(from>mx)
  {
    return;
  }
  if(to==-1 || to>mx)
  {
    to=mx;
  }
  if(size!=mx && autoSize)
  {
    size=mx;
  }

  if(size==0)return;

  if(newvbosize>vbosize)
  {
    if(vbo)
    {
      GLCHK(glDeleteBuffers(1,&vbo));
    }
    GLCHK(glGenBuffers(1,&vbo));
    GLCHK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    vbosize=newvbosize;
    GLCHK(glBufferData(GL_ARRAY_BUFFER, vbosize, 0, GL_DYNAMIC_DRAW));
  }else
  {
    GLCHK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  }
  if(updateFlags&ufVertices)
  {
    GLCHK(glBufferSubData(GL_ARRAY_BUFFER, vbufSize(from),vbufSize(to-from), &vbuf[from]));
  }
  int offset=vbufSize();
  if(tenabled)
  {
    if(updateFlags&ufTexture)
    {
      GLCHK(glBufferSubData(GL_ARRAY_BUFFER, offset+tbufSize(from),tbufSize(to-from), &tbuf[from]));
    }
    offset+=tbufSize();
  }
  if(cenabled && (updateFlags&ufColors))
  {
    GLCHK(glBufferSubData(GL_ARRAY_BUFFER, offset+cbufSize(from),cbufSize(to-from), &cbuf[from]));
  }
  GLCHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

}

}
