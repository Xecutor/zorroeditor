#include "Scissors.hpp"
#include "SysHeaders.hpp"
#include "GLState.hpp"

namespace glider{

void Scissors::draw()
{
  state.enableScissors(rect);
  obj->draw();
  state.disableScissors();
}

void Scissors::draw(Drawable* argObj)
{
  state.enableScissors(rect);
  argObj->draw();
  state.disableScissors();
}

}
