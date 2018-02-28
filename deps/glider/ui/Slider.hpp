/*
 * Slider.hpp
 *
 *  Created on: 26.08.2012
 *      Author: Xecutor
 */

#ifndef __GLIDER_UI_SLIDER_HPP__
#define __GLIDER_UI_SLIDER_HPP__

#include "UIObject.hpp"
#include "../core/Rectangle.hpp"
#include "../core/Line.hpp"

namespace glider{
namespace ui{

class Slider:public UIObject{
public:
  Slider(float argCurValue=0.0,float argMinValue=0.0,float argMaxValue=1.0);
  float getValue()const
  {
    return curValue;
  }
  void setValue(float argValue)
  {
    curValue=argValue;
  }

  void draw();

protected:
  float curValue,minValue,maxValue;
  Rectangle rect;
  Line line;
  bool mouseDown;

  void updateValue(int mx);

  void onMouseButtonDown(const MouseEvent& me);
  void onMouseButtonUp(const MouseEvent& me);
  void onMouseMove(const MouseEvent& me);

};

}
}


#endif /* SLIDER_HPP_ */
