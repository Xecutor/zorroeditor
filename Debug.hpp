#ifndef __ZORRO_DEBUG_HPP__
#define __ZORRO_DEBUG_HPP__

#include <stdio.h>

namespace zorro{

#ifdef DEBUG
extern FILE* dpFile;
#ifdef _MSC_VER
#define DPRINT(...) if(dpFile){fprintf(dpFile,"%s:%d:",__FILE__,__LINE__);fprintf(dpFile,__VA_ARGS__);fflush(dpFile);}
#else
#define DPRINT(...) if(dpFile){fprintf(dpFile,"%s:%d:",__func__,__LINE__);fprintf(dpFile,__VA_ARGS__);fflush(dpFile);}
#endif
#else
#define DPRINT(...)
#endif

}

#endif
