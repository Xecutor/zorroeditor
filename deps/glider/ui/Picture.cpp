#include "Picture.hpp"
#include "GLState.hpp"

namespace glider{
namespace ui{

void Picture::draw()
{
  if(image.get())
  {
    RelOffsetGuard rg(pos);
    image->draw();
  }
}

}
}
