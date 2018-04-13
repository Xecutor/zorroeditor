#include "Texture.hpp"
#include "SysHeaders.hpp"
#include "GLState.hpp"
#include "Engine.hpp"

namespace glider{

Texture::Texture():texId(0),fboId(0)
{
  recreate();
}

void Texture::recreate()
{
  if(texId)
  {
    glDeleteTextures(1,&texId);
  }
  if(fboId)
  {
    glDeleteFramebuffersEXT(1,&fboId);
  }
  fboId=0;
  glGenTextures(1,&texId);
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}


void Texture::setLinearMagFiltering()
{
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
void Texture::setLinearMinFiltering()
{
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
void Texture::setNearestMagFiltering()
{
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
void Texture::setNearestMinFiltering()
{
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void Texture::setClump()
{
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}
void Texture::setRepeat()
{
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void Texture::setMirroredRepeat()
{
  bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}


Texture::~Texture()
{
  glDeleteTextures(1,&texId);
  if(fboId)
  {
    glDeleteFramebuffersEXT(1,&fboId);
  }
}

void Texture::create(int argWidth,int argHeight)
{
  texWidth=argWidth;
  texHeight=argHeight;
  bind();
  GLCHK(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,texWidth,texHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,0));
}

void Texture::loadSurface(SDL_Surface* img)
{
  bind();
  texWidth=img->w;
  texHeight=img->h;
  int format=GL_RGBA;
  if(img->format->BytesPerPixel==3)
  {
    if(img->format->Rmask==0xff0000)
    {
      format=GL_BGR;
    }else
    {
      format=GL_RGB;
    }
  }else
  {
    if(img->format->Rmask==0xff0000)
    {
      format=GL_BGRA;
    }else
    {
      format=GL_RGBA;
    }
  }
  GLCHK(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img->w,img->h,0,format,GL_UNSIGNED_BYTE,img->pixels));
}

void Texture::bind()const
{
  state.enableTexture();
  GLCHK(glBindTexture(GL_TEXTURE_2D,texId));
}

void Texture::unbind()const
{
  glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture::renderHere(Drawable* obj)
{
  if(fboId==0)
  {
    GLCHK(glGenFramebuffersEXT(1,&fboId));
    //printf("fboId=%u\n",fboId);
  }
  GLCHK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId));
  GLCHK(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texId, 0));

  //GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  //printf("status=%d(%d)\n",status,GL_FRAMEBUFFER_COMPLETE_EXT);
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // enable transparency

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  GLCHK(glViewport(0,0,texWidth,texHeight));
  GLCHK(glOrtho(0, texWidth, 0,texHeight, -1.0, 1.0));
  glMatrixMode(GL_MODELVIEW);
  state.loadIdentity();
  obj->draw();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  GLCHK(glViewport(0,0,engine.getWidth(),engine.getHeight()));
  glMatrixMode(GL_MODELVIEW);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void Texture::clear()
{
  /*bind();
  GLuint clearColor = 0;
  GLCHK(glClearTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, &clearColor));*/

  if(fboId==0)
  {
    GLCHK(glGenFramebuffersEXT(1,&fboId));
    //printf("fboId=%u\n",fboId);
  }
  GLCHK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId));
  GLCHK(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0));
  GLenum buffers[] = { GL_COLOR_ATTACHMENT0};
  GLCHK(glDrawBuffers(1,buffers));
  GLint clr[4]={0,0,0,0};
  GLCHK(glClearBufferiv(GL_COLOR,0/*GL_DRAW_BUFFER0*/,clr));
  glClearColor(0,0,0,0);
  GLCHK(glClear(GL_COLOR_BUFFER_BIT));
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  const Color& clearColor=engine.getClearColor();
  glClearColor(clearColor.r,clearColor.g,clearColor.b,clearColor.a);

}


}
