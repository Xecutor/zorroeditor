#include "Rectangle.hpp"
#include "SysHeaders.hpp"
#include "GLState.hpp"

namespace glider{

void Rectangle::draw()
{
  state.loadIdentity();
  state.translate(pos);
  state.disableTexture();
  Rect rect(Pos(),size);
  clr.select();
  if(fill)
  {
    glBegin(GL_QUADS);
    rect.tl().select();
    rect.tr().select();
    rect.br().select();
    rect.bl().select();
  }else
  {
    state.translate(Pos(0.5f,0.5f));
    glBegin(GL_LINES);
    rect.tl().select();
    rect.tr().select();
    rect.tr().select();
    rect.br().select();
    rect.br().select();
    rect.bl().select();
    rect.bl().select();
    rect.tl().select();
  }
  glEnd();
}

}
