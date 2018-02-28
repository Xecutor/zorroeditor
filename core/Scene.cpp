#include "Scene.hpp"

namespace glider{

void Scene::draw()
{
  for(DrawableObjectsList::iterator it=objects.begin(),end=objects.end();it!=end;++it)
  {
    (*it)->draw();
  }
}

DrawableObjectId Scene::addObject(Drawable* argObject)
{
  objects.push_back(argObject);
  return --objects.end();
}

void Scene::removeObject(DrawableObjectId id)
{
  objects.erase(id);
}

void Scene::clear()
{
  objects.clear();
}


}
