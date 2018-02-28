#ifndef __GLIDER_UTIL_MATRIX_HPP__
#define __GLIDER_UTIL_MATRIX_HPP__

#include <sys/types.h>
#include <kst/Throw.hpp>

namespace glider{
namespace util{

template <class T>
class Matrix{
public:
  Matrix():data(0),width(0),height(0){}
  Matrix(size_t argWidth,size_t argHeight):data(0),width(0),height(0)
  {
    resize(argWidth,argHeight);
  }
  ~Matrix()
  {
    clear();
  }
  void resize(int argWidth,int argHeight)
  {
    size_t oldHeight=height;
    T** oldData=data;

    data=new T*[argHeight];
    for(int y=0;y<argHeight;y++)
    {
      data[y]=new T[argWidth];
      if(y<height)
      {
        for(int x=0;x<width && x<argWidth;x++)
        {
          data[y][x]=oldData[y][x];
        }
      }
    }
    width=argWidth;
    height=argHeight;

    if(oldData)
    {
      for(size_t y=0;y<oldHeight;y++)
      {
        delete [] oldData[y];
      }
      delete [] oldData;
    }
  }
  void clear()
  {
    if(data)
    {
      for(int y=0;y<height;y++)
      {
        delete [] data[y];
      }
      delete [] data;
      data=0;
      height=0;
      width=0;
    }
  }
  T& operator()(int x,int y)
  {
    if(x<0 || x>=width || y<0 || y>=height)
    {
      KSTHROW("Matrix access out of range (%d,%d)(%dx%d)",x,y,width,height);
    }
    return data[y][x];
  }
  const T& operator()(int x,int y)const
  {
    if(x<0 || x>=width || y<0 || y>=height)
    {
      KSTHROW("Matrix access out of range (%d,%d)(%dx%d)",x,y,width,height);
    }
    return data[y][x];
  }
  int getWidth()const
  {
    return width;
  }
  int getHeight()const
  {
    return height;
  }
protected:
  T** data;
  int width,height;
};

}
}

#endif
