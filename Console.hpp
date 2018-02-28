#ifndef __ZE_CONSOLE_HPP__
#define __ZE_CONSOLE_HPP__

#include "ui/Window.hpp"
#include "ui/ListBox.hpp"
#include "ZorroVM.hpp"

using namespace glider;
using namespace glider::ui;

class Console:public UIContainer{
public:
  Console();
  void addMsg(const char* msg);
  void registerInZVM(zorro::ZorroVM& vm);
protected:
  ListBox* lb;
  kst::Logger* log;
};

#endif

