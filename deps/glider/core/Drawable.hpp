#ifndef __GLIDER_DRAWTARGET_HPP__
#define __GLIDER_DRAWTARGET_HPP__

#include "Managed.hpp"

namespace glider{

class Drawable:public Managed{
public:
  virtual void draw()=0;
};

typedef ReferenceTmpl<Drawable> DrawableRef;

}

#endif
