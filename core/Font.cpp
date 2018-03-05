#include "Font.hpp"
#include "SysHeaders.hpp"
#ifdef __APPLE__
#include <SDL2_ttf/SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif
#include <memory.h>
#include <kst/Throw.hpp>

namespace glider{

Font::Font()
{
  memset(glyphs,0,sizeof(glyphs));
  font=0;
  img=0;
  texSize=512;
  curX=0;
  curY=0;
  nextY=0;
  //tex.setNearestMagFiltering();
  tex.setLinearMagFiltering();
  tex.setLinearMinFiltering();
}

Font::~Font()
{
  if(font)
  {
    TTF_CloseFont(font);
  }
  for(size_t i=0;i<256;i++)
  {
    if(glyphs[i])
    {
      delete [] glyphs[i];
    }
  }
  if(img)
  {
    SDL_FreeSurface(img);
  }
}

void Font::loadTTF(const char* argFileName,int argSize,int argFlags)
{
  fileName=argFileName;
  size=argSize;
  flags=argFlags;
  font=TTF_OpenFont(argFileName, size);
  if(!font) {
    KSTHROW("Failed to open font file %{}", argFileName);
  }
  if(argFlags&ffHinting)
  {
    TTF_SetFontHinting(font,TTF_HINTING_NORMAL);
  }else
  {
    TTF_SetFontHinting(font,TTF_HINTING_NONE);
  }
  if(argFlags&ffBold)
  {
    TTF_SetFontStyle(font,TTF_STYLE_BOLD);
  }
  tex.bind();
  img=SDL_CreateRGBSurface(0,texSize,texSize,32,0xff0000,0x00ff00,0x0000ff,0xff000000);
  SDL_SetSurfaceBlendMode(img,SDL_BLENDMODE_NONE);
  SDL_SetSurfaceAlphaMod(img,255);
  for(ushort c=32;c<128;c++)
  {
    prepareGlyph(c);
  }
  updateTex();
}

void Font::reload()
{
  if(img)
  {
    tex.recreate();
    updateTex();
  }
}


void Font::updateTex()
{
  tex.bind();
  tex.loadSurface(img);
  //glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,texSize,texSize,0,GL_RGBA,GL_UNSIGNED_BYTE,img->pixels);
}


GlyphInfo& Font::getGIRef(ushort c)
{
  GlyphInfo*& ptr=glyphs[c>>8];
  if(ptr)
  {
    return ptr[c&0xff];
  }
  ptr=new GlyphInfo[256];
  return ptr[c&0xff];
}


bool Font::prepareGlyph(ushort c)
{
  GlyphInfo& gi=getGIRef(c);
  if(gi.init)
  {
    return false;
  }
  SDL_Color clr={255,255,255,255};
  SDL_Surface* tmp=TTF_RenderGlyph_Blended(font,c,clr);
  //SDL_FreeSurface(tmp);
  if(curX+tmp->w>texSize)
  {
    curX=0;
    curY=nextY+1;
  }
  //SDL_SetAlpha(tmp,0,0);
  //SDL_SetSurfaceAlphaMod(tmp,255);
  SDL_SetSurfaceBlendMode(tmp,SDL_BLENDMODE_NONE);
  SDL_Rect src={0,0,tmp->w,tmp->h};
  SDL_Rect dst={curX,curY,tmp->w,tmp->h};
  if(flags&ffGlow)
  {
    dst.x++;
    dst.y++;
  }
  SDL_BlitSurface(tmp,&src,img,&dst);
  if(flags&ffGlow)
  {
    int maxX=tmp->w+2;
    int maxY=tmp->h+2;
    typedef std::vector<Color> ClrVector;
    typedef std::vector<ClrVector> ClrMatrix;
    ClrMatrix m(maxY,ClrVector(maxX,Color(0,0,0,0)));
    for(int y0=0;y0<maxY;++y0)
    {
      uint32_t* pix=(uint32_t*)img->pixels;
      for(int x0=0;x0<maxX;++x0)
      {
        for(int xx=-1;xx<=1;++xx)
        {
          for(int yy=-1;yy<=1;++yy)
          {
            int x=curX+x0+xx;
            int y=curY+y0+yy;
            if(x<curX || x>=curX+maxX || y<curY || y>=curY+maxY || (xx==0 && yy==0))
            {
              continue;
            }
            uint32_t& clr=pix[img->w*y+x];
            if((clr&0xff000000u)==0)clr=0;
            m[y0][x0]+=Color(clr,true);
          }
        }
      }
    }
    for(int y0=0;y0<maxY;++y0)
    {
      uint32_t* pix=(uint32_t*)img->pixels;
      for(int x0=0;x0<maxX;++x0)
      {
        int x=curX+x0;
        int y=curY+y0;
        m[y0][x0]/=8.0f;
        uint32_t& clr=pix[img->w*y+x];
        Color& c=m[y0][x0];
        if((clr&0xff000000u)==0 && c.a!=0)
        {
          clr=(uint32_t)(255*(c.r+c.g+c.b)/3);
          clr<<=24;
          clr|=0xffffff;
        }else
        {
          clr=0;
        }

      }
    }
  }
  int minx,maxx,miny,maxy,adv;
  TTF_GlyphMetrics(font,c,&minx,&maxx,&miny,&maxy,&adv);
  gi.minx=(float)minx;
  gi.maxx=(float)maxx;
  gi.miny=(float)miny;
  gi.maxy=(float)maxy;
  gi.adv=(float)adv;//>0?adv:tmp->w+1;
  gi.xshift=0;
  gi.yshift=0;
  gi.texSize=Pos((float)tmp->w,(float)tmp->h);
  if(flags&ffGlow)
  {
    gi.texSize+=Pos(2,2);
    gi.xshift=-1;
    gi.yshift=-1;
  }

  //printf("%c: minx=%d, maxx=%d, miny=%d, maxy=%d, adv=%d,texSize=(%d,%d)\n",c,minx,maxx,miny,maxy,adv,tmp->w,tmp->h);
  /*for(int y=0;y<tmp->h;++y)
  {
    Uint8* ptr=((Uint8*)tmp->pixels)+(y*tmp->w*4);
    for(int x=0;x<tmp->w;++x)
    {
      printf("%c",ptr[x*4+3]?'#':'.');
    }
    printf("\n");
  }*/
  float fix=1.0f/texSize;
  gi.texPos=Pos(curX*(1.0f-fix)/(texSize-1),curY*(1.0f-fix)/(texSize-1));
  //gi.texSize=Posi<ushort>(tmp->w,tmp->h);//Pos(tmp->w,tmp->h);
  //gi.texPos=Posi<ushort>(curX,curY);//Pos(1.0f*curX/texSize,1.0f*curY/texSize);
  curX+=tmp->w+1+(flags&ffGlow?2:0);
  if(curY+tmp->h+(flags&ffGlow?2:0)>nextY)
  {
    nextY=curY+tmp->h+(flags&ffGlow?2:0);
  }
  gi.init=true;
  SDL_FreeSurface(tmp);
  return true;
}

void Font::prepareForString(const UString& str)
{
  bool updated=false;
  for(int i=0;i<str.getSize();)
  {
    if(prepareGlyph(str.getNext(i)))
    {
      updated=true;
    }
  }
  if(updated)
  {
    updateTex();
  }
}


GlyphInfo Font::getGlyph(ushort glyph)
{
  GlyphInfo& gi=getGIRef(glyph);
  if(!gi.init)
  {
    prepareGlyph(glyph);
    updateTex();
  }
  return gi;
}

int Font::getHeight()const
{
  return TTF_FontHeight(font);
}


int Font::getLineSkip()const
{
  return TTF_FontLineSkip(font);
}

int Font::getAscent()const
{
  return TTF_FontAscent(font);
}

}
