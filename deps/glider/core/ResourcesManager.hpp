#ifndef __GLIDER_RESOURCESMANAGER_HPP__
#define __GLIDER_RESOURCESMANAGER_HPP__

#include "Managed.hpp"
#include "Font.hpp"
#include <string>
#include <map>
#include "SdlFwdDecl.h"

namespace glider{

struct SurfaceHolder:public Managed{
  SDL_Surface* surf;
  SurfaceHolder(SDL_Surface* argSurf=0):surf(argSurf){}
  virtual ~SurfaceHolder();
};

typedef ReferenceTmpl<SurfaceHolder> SurfaceRef;

class ResourcesManager{
public:
  ManagedObjectId addObject(Managed* obj)
  {
    return objList.insert(objList.end(),obj);
  }
  void removeObject(ManagedObjectId id)
  {
    objList.erase(id);
  }
  void reloadAll()
  {
    for(ManagedObjectsList::iterator it=objList.begin(),end=objList.end();it!=end;++it)
    {
      (*it)->reload();
    }
  }

  FontRef getFont(const char* name,int size,int flags=ffHinting);
  SurfaceRef getSurf(const char* name);

protected:
  ManagedObjectsList objList;

  struct FontSpec{
    std::string name;
    int size;
    int flags;
    FontSpec(const std::string& argName,int argSize,int argFlags):name(argName),size(argSize),flags(argFlags){}
    bool operator<(const FontSpec& other)const
    {
      return name<other.name || (name==other.name && size<other.size) || (name==other.name && size==other.size && flags<other.flags);
    }
  };

  typedef std::map<FontSpec,FontRef> FontsMap;
  FontsMap fonts;

  typedef std::map<std::string,SurfaceRef> SurfMap;
  SurfMap surfaces;

};

extern ResourcesManager manager;

}

#endif
