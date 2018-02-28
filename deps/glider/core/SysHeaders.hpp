#ifndef __GLIDER_SYSHEADERS_HPP__
#define __GLIDER_SYSHEADERS_HPP__

#include <SDL2/SDL.h>
#include <GL/glew.h>

inline void glErrorDebug(){}

#ifdef DEBUG
#define GLCHK(x) x;{int err;if((err=glGetError())==0){}else{glErrorDebug();fprintf(stderr,"failed with err=%d at line " __FILE__ ":%d\n",err,__LINE__);}}
#else
#define GLCHK(x) x
#endif


#endif
