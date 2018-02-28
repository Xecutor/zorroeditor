/*
 * Splitter.hpp
 *
 *  Created on: 11 сент. 2016 г.
 *      Author: konst
 */

#ifndef SRC_UI_SPLITTER_HPP_
#define SRC_UI_SPLITTER_HPP_

#include "UIContainer.hpp"
#include "Rectangle.hpp"

namespace glider{
namespace ui{

class Splitter : public UIContainer{
public:
  enum SplitterOrientation{
    soHorizontal,
    soVertical
  };
  enum ResizePolicy{
    rpProportional,
    rpKeepFirst,
    rpKeepSecond
  };
  Splitter(SplitterOrientation argOrientation, int argPosition);
  void setFirst(UIObjectRef obj);
  void setSecond(UIObjectRef obj);
  void setPosition(int argPosition);
  int getPosition()const
  {
    return position;
  }
  void setResizePolicy(ResizePolicy argResizePolicy)
  {
    resizePolicy=argResizePolicy;
  }
  SplitterOrientation getOrientation()const
  {
    return orientation;
  }
protected:
  SplitterOrientation orientation;
  ResizePolicy resizePolicy=rpProportional;
  UIObjectRef first,second;
  struct SplitterRect: public UIObject {
    Rectangle rect;
    void draw()
    {
      rect.draw();
    }
    void onMouseButtonDown(const MouseEvent& me);
    void onMouseButtonUp(const MouseEvent& me);
    void onMouseMove(const MouseEvent& me);
    bool resizing=false;
    Posi<int> resStart;
    int startPos;
  };
  ReferenceTmpl<SplitterRect> sRect;
  int position;
  int splitterWidth;
  int minFirstWidth;
  int minSecondWidth;
  virtual void onObjectResize();
  virtual void onObjectResizeEnd();

  virtual void onParentResize();
  virtual void onParentResizeEnd();

  void recalcPosition();

  void updateFirstPosAndSize();
  void updateSecondPosAndSize();

};

}
}



#endif /* SRC_UI_SPLITTER_HPP_ */
