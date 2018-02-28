#ifndef __GLIDER_MANAGED_HPP__
#define __GLIDER_MANAGED_HPP__

#include <list>

namespace glider{

class Managed;

typedef std::list<Managed*> ManagedObjectsList;
typedef ManagedObjectsList::iterator ManagedObjectId;

class Reference;

class Managed{
public:
  Managed();
  virtual ~Managed();
  virtual void reload(){}
protected:
  ManagedObjectId mngId;
  int refCount;
  friend class Reference;
};

class Reference{
public:
  Reference(Managed* argObj=0):obj(argObj)
  {
    ref();
  }
  Reference(const Reference& argRef):obj(argRef.obj)
  {
    ref();
  }
  ~Reference()
  {
    unref();
  }
  Reference& operator=(Managed* argObj)
  {
    if(obj!=argObj)
    {
      unref();
      obj=argObj;
      ref();
    }
    return *this;
  }
  Reference& operator=(const Reference& argRef)
  {
    if(this==&argRef)
    {
      return *this;
    }
    unref();
    obj=argRef.obj;
    ref();
    return *this;
  }
  void forceRef()
  {
    ref();
  }
protected:
  mutable Managed* obj;
  void ref()const
  {
    if(obj)
    {
      obj->refCount++;
    }
  }
  void unref()const
  {
    if(obj)
    {
      obj->refCount--;
      if(obj->refCount==0)
      {
        delete obj;
        obj=0;
      }
    }
  }
};

template <class T>
class ReferenceTmpl:public Reference{
public:
  ReferenceTmpl(T* argObj=0):Reference(argObj){}
  ReferenceTmpl(const ReferenceTmpl& argRef):Reference(argRef){}
  ReferenceTmpl& operator=(T* argObj)
  {
    Reference::operator=(argObj);
    return *this;
  }
  ReferenceTmpl& operator=(const Reference& argRef)
  {
    Reference::operator=(argRef);
    return *this;
  }
  bool operator==(const ReferenceTmpl& rhs)const
  {
    return obj==rhs.obj;
  }
  bool operator!=(const ReferenceTmpl& rhs)const
  {
    return obj!=rhs.obj;
  }
  T* get()
  {
    return (T*)obj;
  }
  const T* get()const
  {
    return (const T*)obj;
  }
  T* operator->()
  {
    return (T*)obj;
  }
  const T* operator->()const
  {
    return (const T*)obj;
  }
  T& operator*()
  {
    return *(T*)obj;
  }
  const T& operator*()const
  {
    return *(const T*)obj;
  }
  template <class U>
  U* as()
  {
    return dynamic_cast<U*>(obj);
  }
  template <class U>
  const U* as()const
  {
    return dynamic_cast<const U*>(obj);
  }
};

}

#endif
