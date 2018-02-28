#include "Utility.hpp"
#include "SysHeaders.hpp"

namespace glider{

void Pos::select()
{
  glVertex2f(x,y);
}

void Pos::selectTex()
{
  glTexCoord2f(x,y);
}

template <>
void Posi<unsigned short>::select()
{
  glVertex2s(x,y);
}

template <>
void Posi<unsigned int>::select()
{
  glVertex2i(x,y);
}

template <>
void Posi<unsigned short>::selectTex()
{
  glTexCoord2s(x,y);
}

template <>
void Posi<unsigned int>::selectTex()
{
  glTexCoord2i(x,y);
}



void Color::select()
{
  glColor4f(r,g,b,a);

}

Color Color::red(1,0,0);
Color Color::green(0,1,0);
Color Color::blue(0,0,1);
Color Color::white(1,1,1);
Color Color::black(0,0,0);
Color Color::gray(0.5,0.5,0.5);
Color Color::yellow(1,1,0);
Color Color::cyan(0,1,1);

}
