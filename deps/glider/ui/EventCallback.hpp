#ifndef __GLIDER_UI_EVENTCALLBACK_HPP__
#define __GLIDER_UI_EVENTCALLBACK_HPP__

namespace glider{
namespace ui{

template <class ARG>
class EventCallback{
public:
  EventCallback():obj(0),func(0){}
  EventCallback(const EventCallback<ARG>& argOther):obj(argOther.obj),func(argOther.func){}
  void execute(const ARG& arg)
  {
    if(func)func(obj,arg);
  }

  bool isAssigned()const
  {
    return func!=0;
  }

  template <class T,void (T::*METHODPTR)(const ARG&)>
  void setObject(T* argObj)
  {
    obj=argObj;
    func=&Helper<T,METHODPTR>::exec;
  }

  void setFunc(void* argData,void (*argFunc)(void*,const ARG& arg))
  {
    obj=argData;
    func=argFunc;
  }

protected:
  template <class T,void (T::*METHODPTR)(const ARG&)>
  struct Helper{
    static void exec(void* obj,const ARG& arg)
    {
      (((T*)obj)->*METHODPTR)(arg);
    }
  };

  void* obj;
  void (*func)(void*,const ARG& arg);
};

template <>
class EventCallback<void>{
public:
  EventCallback():obj(0),func(0){}
  void execute()
  {
    if(func)func(obj);
  }

  bool isAssigned()const
  {
    return func!=0;
  }

  template <class T,void (T::*METHODPTR)()>
  void setObject(T* argObj)
  {
    obj=argObj;
    func=&Helper<T,METHODPTR>::exec;
  }

  void setFunc(void* argData,void (*argFunc)(void*))
  {
    obj=argData;
    func=argFunc;
  }

protected:
  template <class T,void (T::*METHODPTR)()>
  struct Helper{
    static void exec(void* obj)
    {
      (((T*)obj)->*METHODPTR)();
    }
  };

  void* obj;
  void (*func)(void*);
};

template <class T,class ARG,void (T::*METHODPTR)(const ARG&)>
EventCallback<ARG> mkEventCallback(T* obj)
{
  EventCallback<ARG> cb;
  cb.template setObject<T,METHODPTR>(obj);
  return cb;
}

template <class T,void (T::*METHODPTR)()>
EventCallback<void> mkEventCallback(T* obj)
{
  EventCallback<void> cb;
  cb.template setObject<T,METHODPTR>(obj);
  return cb;
}

template <class ARG>
EventCallback<ARG> mkEventCallback(void* data,void (*funcPtr)(void*,const ARG&))
{
  EventCallback<ARG> cb;
  cb.setFunc(data,funcPtr);
  return cb;
}




#define MKCALLBACK(type,name) mkEventCallback<ThisClass,type,&ThisClass::name>(this)
#define MKCALLBACK0(name) mkEventCallback<ThisClass,&ThisClass::name>(this)


}
}

#endif
