#ifndef __GLIDER_SCENE_HPP__
#define __GLIDER_SCENE_HPP__

#include "Drawable.hpp"
#include <list>

namespace glider{

typedef std::list<DrawableRef> DrawableObjectsList;
typedef DrawableObjectsList::iterator DrawableObjectId;

class Scene:public Drawable{
public:
  void draw();
  DrawableObjectId addObject(Drawable* argObject);
  void removeObject(DrawableObjectId id);
  void clear();
  DrawableObjectId begin()
  {
    return objects.begin();
  }
  DrawableObjectId end()
  {
    return objects.end();
  }
protected:
  DrawableObjectsList objects;
};

}

#endif
