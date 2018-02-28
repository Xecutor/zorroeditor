#ifndef __GLIDER_GLSTATE_HPP__
#define __GLIDER_GLSTATE_HPP__

#include "Utility.hpp"
#include <vector>

namespace glider{

class GLState{
public:
  GLState();
  void enableTexture();
  void disableTexture();
  void enableBlend();
  void disableBlend();
  void loadIdentity();
  void translate(const Pos& off);
  void rotate(float angle);
  void scale(const Pos& rate);
  void enableScissors(const Rect& argRect,bool texMode=false);
  void disableScissors();
  void pushOffset(const Pos& off);
  void pushRelOffset(const Pos& off);
  void popOffset();
  void disableOffset();
  void enableOffset();
  void setLineStipple(int pixPerBit,ushort pattern);
  void disableLineStipple();

  const char* getRenderer();
  const char* getVendor();

  Posi<int> getScreenRes();

protected:
  bool blendEnabled;
  bool textureEnabled;
  bool identityChanged;
  bool stippleEnabled;
  std::vector<Rect> scissorsStack;
  std::vector<Pos> offsetsStack;
};

extern GLState state;

class OffsetGuard{
public:
  OffsetGuard(const Pos& off)
  {
    state.pushOffset(off);
  }
  ~OffsetGuard()
  {
    state.popOffset();
  }
};

class RelOffsetGuard{
public:
  RelOffsetGuard(const Pos& off)
  {
    state.pushRelOffset(off);
  }
  ~RelOffsetGuard()
  {
    state.popOffset();
  }
};

class ScissorsGuard{
public:
  ScissorsGuard(const Rect& rect)
  {
    state.enableScissors(rect);
  }
  ~ScissorsGuard()
  {
    state.disableScissors();
  }
};

}

#endif
