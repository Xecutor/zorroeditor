#include "ResourcesManager.hpp"
#include "SysHeaders.hpp"
#ifdef __APPLE__
#include <SDL2_image/SDL_image.h>
#else
#include <SDL2/SDL_image.h>
#endif

namespace glider{

ResourcesManager manager;

SurfaceHolder::~SurfaceHolder()
{
  if(surf)
  {
    SDL_FreeSurface(surf);
  }

}

FontRef ResourcesManager::getFont(const char* name,int size,int flags)
{
  FontsMap::iterator it=fonts.find(FontSpec(name,size,flags));
  if(it!=fonts.end())
  {
    return it->second;
  }
  Font* rv=new Font();
  fonts.insert(FontsMap::value_type(FontSpec(name,size,flags),rv));
  rv->loadTTF(name,size,flags);
  return rv;
}

SurfaceRef ResourcesManager::getSurf(const char* name)
{
  SurfMap::iterator it=surfaces.find(name);
  if(it!=surfaces.end())
  {
    return it->second;
  }
  SDL_Surface* surf=IMG_Load(name);
  SurfaceHolder* rv=new SurfaceHolder(surf);
  surfaces.insert(SurfMap::value_type(name,rv));
  return rv;

}


}
