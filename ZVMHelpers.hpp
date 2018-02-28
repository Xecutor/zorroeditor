#ifndef __ZE_ZVMHELPERS_HPP__

#include "ZorroVM.hpp"
#include <kst/Throw.hpp>
#include <string>

using namespace zorro;

inline void expectArgs(ZorroVM* vm,const char* funcName,int minCount,int maxCount=-1)
{
  int ac=vm->getArgsCount();
  if(ac<minCount || (maxCount>=0 && ac>maxCount))
  {
    KSTHROW("Invalid number of arguments for %{}",funcName);
  }
}

inline int getIntValue(ZorroVM* vm,const char* name,int idx)
{
  Value& val=vm->getLocalValue(idx);
  if(val.vt!=vtInt)
  {
    Value& f=vm->symbols.globals[vm->ctx.callStack.stackTop->funcIdx];
    std::string fn=f.func->name.val.c_str();
    KSTHROW("Argument %{} of function or method %{} must be int, but %s passed",name,fn,getValueTypeName(val.vt));
  }
  return val.iValue;
}

inline std::string getStrValue(ZorroVM* vm,const char* name,int idx)
{
  Value& val=vm->getLocalValue(idx);
  if(val.vt!=vtString)
  {
    Value& f=vm->symbols.globals[vm->ctx.callStack.stackTop->funcIdx];
    std::string fn=f.func->name.val.c_str();
    KSTHROW("Argument %{} of function or method %{} must be string, but %s passed",name,fn,getValueTypeName(val.vt));
  }
  return val.str->c_str(vm);
}


#endif
