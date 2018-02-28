#include "SysHeaders.hpp"
#include "Engine.hpp"

extern int GliderAppMain(int argc,char* argv[]);

extern "C"
int main(int argc,char* argv[])
{
  glider::engine.init();
  int rc=GliderAppMain(argc,argv);
  SDL_Quit();
  return rc;
}
