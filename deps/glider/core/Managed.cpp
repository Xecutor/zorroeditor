#include "Managed.hpp"
#include "ResourcesManager.hpp"

namespace glider{

Managed::Managed():refCount(0)
{
  mngId=manager.addObject(this);
}

Managed::~Managed()
{
  manager.removeObject(mngId);
}

}
