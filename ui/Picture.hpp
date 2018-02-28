#ifndef __GLIDER_UI_PICTURE_HPP__
#define __GLIDER_UI_PICTURE_HPP__

#include "UIObject.hpp"
#include "Image.hpp"

namespace glider{
namespace ui{

class Picture:public UIObject{
public:
  void draw();
  void assignImage(Image* argImage)
  {
    image=argImage;
  }
  Image& getImage()
  {
    return *image;
  }
  const Image& getImage()const
  {
    return *image;
  }
protected:
  ImageRef image;
};

typedef ReferenceTmpl<Picture> PictureRef;

}
}

#endif
