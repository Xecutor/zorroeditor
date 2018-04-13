#include "Image.hpp"
#include "SysHeaders.hpp"
#ifdef __APPLE__
#include <SDL2_image/SDL_image.h>
#else
#include <SDL2/SDL_image.h>
#endif
#include "GLState.hpp"
#include "ResourcesManager.hpp"
#include <kst/Throw.hpp>

namespace glider{

Image::Image()
{
  angle=0.0f;
  scaleX=1.0f;
  scaleY=1.0f;
  texRect=Rect(Pos(0,0),Pos(1.0f,1.0f));
}

Image::~Image()
{
}


void Image::draw()
{
//  state.disableOffset();
  state.loadIdentity();
  state.translate(pos);
  if(angle!=0.0f)
  {
    state.rotate(angle);
  }
  if(scaleX!=1.0f || scaleY!=1.0f)
  {
    state.scale(Pos(scaleX,scaleY));
  }
  state.translate(-centerPos);
//  state.enableOffset();
  state.enableTexture();
  color.select();
  Rect scrRect(Pos(0,0),size);
  bind();
  glBegin(GL_QUADS);
  texRect.tl().selectTex();
  scrRect.tl().select();
  texRect.tr().selectTex();
  scrRect.tr().select();
  texRect.br().selectTex();
  scrRect.br().select();
  texRect.bl().selectTex();
  scrRect.bl().select();
  glEnd();
}

static bool isPowerOf2(int x)
{
  if(!x)return 0;
  while((x&1)==0)x>>=1;
  return x==1;
}

void Image::load(const char* argFileName)
{
  surf=manager.getSurf(argFileName);
  SDL_Surface* image=surf->surf;
  if(!image)
  {
    KSTHROW("failed to load image:'%s'",argFileName);
  }
  //printf("%d,%d,%d(%x,%x,%x),al=%d\n",image->w,image->h,image->format->BytesPerPixel,image->format->Rmask,image->format->Gmask,image->format->Bmask,image->format->alpha);
  size=Pos((float)image->w,(float)image->h);
  centerPos=Pos(size.x/2.0f,size.y/2.0f);
  if(image->format->BytesPerPixel<=3 || !isPowerOf2(image->w) || !isPowerOf2(image->h))
  {
    int w=image->w;
    int h=image->h;
    int b=1;
    for(int i=0;i<31;i++)
    {
      if(w==b)
      {
        break;
      }
      if(b>w)
      {
        w=b;
        break;
      }
      b<<=1;
    }
    b=1;
    for(int i=0;i<31;i++)
    {
      if(h==b)
      {
        break;
      }
      if(b>h)
      {
        h=b;
        break;
      }
      b<<=1;
    }
    uint rmask=0xff0000;
    uint gmask=0x00ff00;
    uint bmask=0x0000ff;
    uint amask=0xff000000;
    if(image->format->BytesPerPixel>=3)
    {
      rmask=image->format->Rmask;
      gmask=image->format->Gmask;
      bmask=image->format->Bmask;
      amask=image->format->Amask;
    }
    int flags=0;
    int bpp=image->format->BytesPerPixel==4/* || (image->flags&SDL_SRCCOLORKEY)*/?32:24;
    texRect.size=Pos(1.0f*image->w/w,1.0f*image->h/h);
    //SDL_SetAlpha(image,0,0);
    SDL_Surface* tmpImage=SDL_CreateRGBSurface(flags,w,h,bpp,rmask,gmask,bmask,amask);
    SDL_BlitSurface(image,0,tmpImage,0);
    SDL_FreeSurface(image);
    surf->surf=tmpImage;
  }
  reload();
}

void Image::reload()
{
  recreate();
  //bind();
  update();
}

void Image::update()
{
  loadSurface(surf->surf);
}


void Image::create(int argWidth,int argHeight,bool allocate)
{
  Texture::create(argWidth,argHeight);
  size=Pos((float)argWidth,(float)argHeight);
  //centerPos=Pos(size.x/2.0f,size.y/2.0f);
  if(allocate)
  {
    uint rmask=0xff0000;
    uint gmask=0x00ff00;
    uint bmask=0x0000ff;
    uint amask=0xff000000;
    int flags=0;//SDL_SRCALPHA;
    int bpp=32;
    SDL_Surface* tmp=SDL_CreateRGBSurface(flags,argWidth,argHeight,bpp,rmask,gmask,bmask,amask);
    surf=new SurfaceHolder(tmp);
  }
}

Color Image::getPixel(int x,int y)
{
  if(!surf.get())
  {
    return Color();
  }
  SDL_Surface& s=*surf->surf;
  unsigned char* px=(unsigned char*)surf->surf->pixels;
  px+=y*s.w*s.format->BytesPerPixel;
  px+=x*s.format->BytesPerPixel;
  uint32_t& pix=*((uint32_t*)px);
  Color rv;
  rv.r=1.0f*((pix&s.format->Rmask)>>s.format->Rshift)/255.0f;
  rv.g=1.0f*((pix&s.format->Gmask)>>s.format->Gshift)/255.0f;
  rv.b=1.0f*((pix&s.format->Bmask)>>s.format->Bshift)/255.0f;
  if(s.format->Amask)
  {
    rv.a=1.0f*((pix&s.format->Amask)>>s.format->Ashift)/255.0f;
  }else
  {
    rv.a=1.0;
  }
  return rv;
}

void Image::setPixel(int x,int y,Color clr)
{
  if(!surf.get())
  {
    return;
  }
  SDL_Surface& s=*surf->surf;
  unsigned char* px=(unsigned char*)surf->surf->pixels;
  px+=y*s.w*s.format->BytesPerPixel;
  px+=x*s.format->BytesPerPixel;
  uint32_t& pix=*((uint32_t*)px);
  unsigned char r=(unsigned char)(clr.r*255);
  unsigned char g=(unsigned char)(clr.g*255);
  unsigned char b=(unsigned char)(clr.b*255);
  unsigned char a=(unsigned char)(clr.a*255);

  pix=0;
  pix|=r<<s.format->Rshift;
  pix|=g<<s.format->Gshift;
  pix|=b<<s.format->Bshift;
  if(s.format->Ashift)
  {
    pix|=a<<s.format->Ashift;
  }
}


/*
unsigned char* Image::getPixels()
{
  if(surf.get())
  {
    return (unsigned char*)surf->surf->pixels;
  }else
  {
    return 0;
  }
}

int Image::getBytesPerPixel()
{
  if(surf.get())
  {
    return surf->surf->format->BytesPerPixel;
  }else
  {
    return 0;
  }
}

*/

}
