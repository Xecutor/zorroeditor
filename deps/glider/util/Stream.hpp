#ifndef __GLIDER_UTIL_STREAM_HPP__
#define __GLIDER_UTIL_STREAM_HPP__

#include <sys/types.h>

namespace glider{
namespace util{

class Stream{
public:
  virtual ~Stream(){}
  virtual size_t Read(void* buf,size_t sz)=0;
  virtual size_t Write(const void* buf,size_t sz)=0;
  virtual void Skip(size_t sz)=0;
  virtual bool isEof()=0;
};

}
}


#endif
