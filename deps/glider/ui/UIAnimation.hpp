#ifndef __GLIDER_UI_UIANIMATION_HPP__
#define __GLIDER_UI_UIANIMATION_HPP__

#include "Managed.hpp"

namespace glider{
namespace ui{

class UIAnimation:public Managed{
public:
  virtual void onStart(){}
  virtual void onEnd(){}
  virtual bool update(int mcsec)=0;
  virtual bool deleteOnFinish()
  {
    return true;
  }
};

}
}

#endif
