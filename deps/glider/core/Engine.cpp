#include "Engine.hpp"
#include "SysHeaders.hpp"
#ifdef __APPLE__
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#else
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#endif
#ifndef _WIN32
#include <unistd.h>
#endif
#include "GLState.hpp"
#include "Timer.hpp"
#include <kst/Logger.hpp>

namespace glider{

Engine engine;

Engine::Engine()
{
  initialized=false;
  flags=0;
  flags |= SDL_WINDOW_OPENGL;
  //flags |= SDL_GL_DOUBLEBUFFER;
  screenWidth=1024;
  screenHeight=768;
  bpp=32;
  fullScreen=false;
  vsync=false;
  loopExitFlag=false;
  appExitFlag=false;
  handler=0;
  limitFps=false;
  targetFps=0;
  clearColor=Color(0,0,0,1);
  screen=0;
}

void Engine::enableResizable()
{
  flags |= SDL_WINDOW_RESIZABLE;
}

void Engine::selectResolution(int argWidth,int argHeight,bool argFullScreen)
{
  if(argFullScreen)
  {
    flags|=SDL_WINDOW_FULLSCREEN;
  }else
  {
    flags&=~SDL_WINDOW_FULLSCREEN;
  }
  screenWidth=argWidth;
  screenHeight=argHeight;
}

void Engine::updateResolution(int argWidth,int argHeight)
{
  screenWidth=argWidth;
  screenHeight=argHeight;
  glViewport(0, 0, screenWidth, screenHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, screenWidth, screenHeight, 0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  //screen->w=argWidth;
  //screen->h=argHeight;
}

static void setGLattr(SDL_GLattr attr,int val)
{
  if(SDL_GL_SetAttribute(attr,val)!=0)
  {
    fprintf(stderr,"failed to set attr %d\n",attr);
  }
}

void Engine::setResolution()
{
  screen = SDL_CreateWindow("",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,screenWidth, screenHeight, flags);
  if(!screen)
  {
    fprintf(stderr,"error:%s\n",SDL_GetError());
    exit(-1);
  }
  SDL_GLContext glcontext = SDL_GL_CreateContext(screen);
  glewInit();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glClearColor(clearColor.r,clearColor.g,clearColor.b,clearColor.a);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
  updateResolution(screenWidth,screenHeight);
}

void Engine::init()
{
  //setenv("SDL_VIDEO_CENTERED","1",true);
  //char var[]="SDL_VIDEO_CENTERED=1";
  //putenv(var);
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();
  IMG_Init(IMG_INIT_JPG | IMG_INIT_TIF | IMG_INIT_PNG);
  bool gl_doublebuf=true;
  setGLattr(SDL_GL_RED_SIZE, 8);
  setGLattr(SDL_GL_GREEN_SIZE, 8);
  setGLattr(SDL_GL_BLUE_SIZE, 8);
  //setGLattr(SDL_GL_DEPTH_SIZE, bpp);
  setGLattr(SDL_GL_DOUBLEBUFFER, gl_doublebuf);
  SDL_GL_SetSwapInterval(vsync);
  setGLattr(SDL_GL_ACCELERATED_VISUAL,1);
  /*int val=-1;
  SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL,&val);
  fprintf(stderr,"acc:%d\n",val);*/
  //SDL_EnableUNICODE(1);

  initialized=true;
}

void Engine::setVSync(bool val)
{
  vsync=val;
  if(initialized)
  {
    SDL_GL_SetSwapInterval(vsync);
  }
}


void Engine::setCaption(const char* argTitle)
{
  SDL_SetWindowTitle(screen,argTitle);

}

void Engine::setClearColor(const Color& argColor)
{
  clearColor=argColor;
  glClearColor(clearColor.r,clearColor.g,clearColor.b,clearColor.a);
}


void Engine::setIcon(const char* argFileName)
{
  SDL_Surface* image=IMG_Load(argFileName);
  SDL_SetWindowIcon(screen,image);
  SDL_FreeSurface(image);
}

void Engine::draw(Drawable* obj)
{
  beginFrame();
  obj->draw();
  endFrame();
}

void Engine::beginFrame()
{
  glClear(GL_COLOR_BUFFER_BIT);
  state.loadIdentity();
}

void Engine::endFrame()
{
//  glFlush();
  //glFinish();
  SDL_GL_SwapWindow(screen);
}

void Engine::enableKeyboardRepeat(int delay,int interval)
{
  //SDL_GetKeyboardState()(delay, interval);
}

void Engine::loop(Drawable* obj)
{
  SDL_Event event;
  Timer t;
  int32_t lastMouseX,lastMouseY;
  int64_t delay=limitFps && targetFps>0?1000000/targetFps:0;
  int64_t overDelay=0;
  Timer frameTimer;
  Timer pt;
  frameTimer.Start();
  static kst::Logger* log=kst::Logger::getLogger("perf");
  while(!loopExitFlag && !appExitFlag)
  {
    if(limitFps && targetFps>0)
    {
      t.Start();
    }
    bool haveEvent;
    pt.Start();
    if(limitFps && targetFps==0)
    {
      haveEvent=SDL_WaitEvent(&event)!=0;
    }else
    {
      haveEvent=SDL_PollEvent(&event)!=0;
    }
    pt.Finish();
    LOGDEBUG(log,"poll %{} mcs",pt.GetMcs());
    while(haveEvent && !appExitFlag)
    {
      switch(event.type)
      {
        case SDL_WINDOWEVENT:
        {
          if(handler)
          {
            if(event.window.event==SDL_WINDOWEVENT_LEAVE || event.window.event==SDL_WINDOWEVENT_ENTER)
            {
              handler->onActiveChange(event.window.event==SDL_WINDOWEVENT_ENTER);
            }else if(event.window.event==SDL_WINDOWEVENT_RESIZED)
            {
              int neww,newh;
              SDL_GetWindowSize(screen,&neww,&newh);
              updateResolution(neww,newh);
              if(handler)
              {
                handler->onResize();
              }
            }
          }
        }break;
        case SDL_QUIT:
        {
          if(handler)
          {
            handler->onQuit();
          }
          else
          {
            appExitFlag=true;
          }
        }break;
        case SDL_MOUSEWHEEL:
        {
          if(handler)
          {
            MouseEvent me;
            me.eventType=metScroll;
            me.buttonsState=0;
            me.xRel=event.wheel.x;
            me.yRel=event.wheel.y;
            me.deviceIndex=event.wheel.which;
            me.eventButton=0;
            me.x=lastMouseX;
            me.y=lastMouseY;
            handler->onMouseEvent(me);
          }
        }break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
          if(handler)
          {
            MouseEvent me;
            me.eventType=event.type==SDL_MOUSEBUTTONDOWN?metButtonPress:metButtonRelease;
            me.buttonsState=event.button.state;
            me.deviceIndex=event.button.which;
            me.eventButton=event.button.button;
            me.x=event.button.x;
            me.y=event.button.y;
            me.xRel=0;
            me.yRel=0;
            handler->onMouseEvent(me);
          }
        }break;
        case SDL_MOUSEMOTION:
        {
          lastMouseX=event.motion.x;
          lastMouseY=event.motion.y;
          if(handler)
          {
            MouseEvent me;
            me.eventType=metMove;
            me.buttonsState=event.motion.state;
            me.eventButton=0;
            me.deviceIndex=event.motion.which;
            me.x=event.motion.x;
            me.y=event.motion.y;
            me.xRel=event.motion.xrel;
            me.yRel=event.motion.yrel;
            handler->onMouseEvent(me);
          }
        }break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
          if(handler)
          {
            KeyboardEvent ke;
            ke.eventType=event.type==SDL_KEYDOWN?ketPress:ketRelease;
            ke.keyMod=(keyboard::KeyModifier)event.key.keysym.mod;
            ke.keySym=keyboard::scanCodeToKeySymbol(event.key.keysym.scancode);
            ke.unicodeSymbol=event.key.keysym.sym;
            ke.repeat=event.key.repeat!=0;
            handler->onKeyboardEvent(ke);
          }
        }break;
        case SDL_TEXTINPUT:
        {
          if(handler)
          {
            KeyboardEvent ke;
            ke.eventType=ketInput;
            ke.keyMod=keyboard::GK_MOD_NONE;
            ke.keySym=keyboard::GK_UNKNOWN;
            int pos=0;
            while(event.text.text[pos]!=0)
            {
              ke.unicodeSymbol=UString::getNextSymbol(event.text.text,pos);
              handler->onKeyboardEvent(ke);
            }
          }
        }break;
        case SDL_USEREVENT:
        {
          if(handler)
          {
            handler->onUserEvent(event.user.data1,event.user.data2);
          }
        }break;
      }
      if(limitFps && targetFps==0)
      {
        haveEvent=SDL_WaitEvent(&event)!=0;
      }else
      {
        haveEvent=SDL_PollEvent(&event)!=0;
      }
    }
    if(handler)
    {
      frameTimer.Finish();
      pt.Start();
      handler->onFrameUpdate((int)frameTimer.GetMcs());
      pt.Finish();
      LOGDEBUG(log,"opTime=%{} mcs",pt.GetMcs());
      frameTimer.Start();
    }
    pt.Start();
    engine.draw(obj);
    pt.Finish();
    LOGDEBUG(log,"engine.draw %{} mcs",pt.GetMcs());
    if(limitFps && targetFps>0)
    {
      t.Finish();
      int64_t opTime=t.GetMcs();
      if(opTime>delay*2)opTime=delay;
      if(delay>opTime+overDelay)
      {
        int64_t toSleep=delay-opTime-overDelay;
        t.Start();
        SDL_Delay((Uint32)(toSleep/1000));
        t.Finish();
        overDelay=t.GetMcs()-toSleep;
      }else
      {
        overDelay-=delay-opTime;
        if(overDelay < -delay)
        {
          overDelay=-delay;
        }
      }
    }
  }
  loopExitFlag=false;
}


}
