#include "GLState.hpp"
#include "SysHeaders.hpp"
#include "Engine.hpp"

namespace glider{

GLState state;

GLState::GLState()
{
  textureEnabled=false;
  identityChanged=false;
  blendEnabled=true;
  stippleEnabled=false;
  scissorsStack.reserve(16);
  offsetsStack.reserve(16);
}

void GLState::enableTexture()
{
  if(textureEnabled)
  {
    return;
  }
  glEnable(GL_TEXTURE_2D);
  textureEnabled=true;
}

void GLState::disableTexture()
{
  if(!textureEnabled)
  {
    return;
  }
  glDisable(GL_TEXTURE_2D);
  textureEnabled=false;
}

void GLState::loadIdentity()
{
  if(!identityChanged)
  {
    return;
  }
  glLoadIdentity();
  if(!offsetsStack.empty())
  {
    glTranslatef(offsetsStack.back().x,offsetsStack.back().y,0);
  }
}


void GLState::translate(const Pos& off)
{
  glTranslatef(off.x,off.y,0);
  identityChanged=true;
}

void GLState::rotate(float angle)
{
  glRotatef(angle,0,0,1.0f);
  identityChanged=true;
}

void GLState::scale(const Pos& rate)
{
  glScalef(rate.x,rate.y,0);
  identityChanged=true;
}

void GLState::enableBlend()
{
  if(blendEnabled)
  {
    return;
  }
  glEnable(GL_BLEND);
}

void GLState::disableBlend()
{
  if(!blendEnabled)
  {
    return;
  }
  glDisable(GL_BLEND);
}

void GLState::enableScissors(const Rect& rect,bool texMode)
{
  GLCHK(glEnable(GL_SCISSOR_TEST));
  float x=rect.pos.x;
  float y=rect.pos.y;
  float w=rect.size.x;
  float h=rect.size.y;
  if(!offsetsStack.empty())
  {
    x+=offsetsStack.back().x;
    y+=offsetsStack.back().y;
  }
  if(!scissorsStack.empty())
  {
    Rect& r=scissorsStack.back();
    if(r.pos.x>x)
    {
      w-=r.pos.x-x;
      x=r.pos.x;
    }
    if(r.pos.y>y)
    {
      h-=r.pos.y-y;
      y=r.pos.y;
    }
    if(w<0)w=0;
    if(h<0)h=0;
    if(x+w>r.pos.x+r.size.x)
    {
      w=r.pos.x+r.size.x-x;
    }
    if(y+h>r.pos.y+r.size.y)
    {
      h=r.pos.y+r.size.y-y;
    }
  }
  scissorsStack.push_back(Rect(x,y,w,h));
  if(!texMode)
  {
    y=engine.getHeight()-y-h;
  }
  GLCHK(glScissor((GLint)x,(GLint)y,(GLsizei)w,(GLsizei)h));
}


void GLState::disableScissors()
{
  scissorsStack.pop_back();
  if(!scissorsStack.empty())
  {
    Rect& rect=scissorsStack.back();
    glScissor((GLint)rect.pos.x,(GLint)(engine.getHeight()-rect.pos.y-rect.size.y),(GLsizei)rect.size.x,(GLsizei)rect.size.y);
  }else
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void GLState::pushRelOffset(const Pos& off)
{
  if(offsetsStack.empty())
  {
    offsetsStack.push_back(off);
  }else
  {
    offsetsStack.push_back(off+offsetsStack.back());
  }
  identityChanged=true;
}

void GLState::pushOffset(const Pos& off)
{
  offsetsStack.push_back(off);
  identityChanged=true;
}

void GLState::popOffset()
{
  if(!offsetsStack.empty())
  {
    identityChanged=true;
    offsetsStack.pop_back();
  }
}

void GLState::setLineStipple(int pixPerBit,ushort pattern)
{
  if(!stippleEnabled)
  {
    GLCHK(glEnable(GL_LINE_STIPPLE));
  }
  GLCHK(glLineStipple(pixPerBit,pattern));
  stippleEnabled=true;
}

void GLState::disableLineStipple()
{
  if(stippleEnabled)
  {
    glDisable(GL_LINE_STIPPLE);
  }
  stippleEnabled=false;
}

const char* GLState::getRenderer()
{
  return (const char*)glGetString(GL_RENDERER);
}

const char* GLState::getVendor()
{
  return (const char*)glGetString(GL_VENDOR);
}

Posi<int> GLState::getScreenRes()
{
  SDL_DisplayMode mode;
  SDL_GetCurrentDisplayMode(0,&mode);
  return Posi<int>(mode.w,mode.h);
}


}
