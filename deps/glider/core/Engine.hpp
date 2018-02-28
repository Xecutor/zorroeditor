#ifndef __GLIDER_ENGINE_HPP__
#define __GLIDER_ENGINE_HPP__

#include "Drawable.hpp"
#include "EventHandler.hpp"
#include "Font.hpp"

struct SDL_Window;

namespace glider{

class Engine{
public:
  Engine();
  void selectResolution(int argWidth,int argHeight,bool argFullScreen);
  void updateResolution(int argWidth,int argHeight);
  void setResolution();
  void init();
  void setCaption(const char* argTitle);
  void setIcon(const char* argFileName);
  void enableResizable();

  void draw(Drawable* obj);

  int getWidth()
  {
    return screenWidth;
  }

  int getHeight()
  {
    return screenHeight;
  }

  void setVSync(bool val=true);
  void enableKeyboardRepeat(int delay=500,int interval=30);

  void loop(Drawable* obj);
  void exitLoop()
  {
    loopExitFlag=true;
  }
  void exitApp()
  {
    appExitFlag=true;
  }

  bool isAppExit()const
  {
    return appExitFlag;
  }

  bool isLoopExit()const
  {
    return loopExitFlag;
  }

  void assignHandler(EventHandler* argHandler)
  {
    handler=argHandler;
  }

  void setFpsLimit(int argTargetFps)
  {
    if(argTargetFps>=0)
    {
      setVSync(false);
      targetFps=argTargetFps;
      limitFps=true;
    }else
    {
      limitFps=false;
    }
  }

  void setClearColor(const Color& argColor);

  const Color& getClearColor()const
  {
    return clearColor;
  }

  void setDefaultFont(FontRef fnt)
  {
    defaultFont=fnt;
  }

  FontRef getDefaultFont()
  {
    return defaultFont;
  }


protected:
  bool initialized;
  int flags;
  int screenWidth;
  int screenHeight;
  Color clearColor;
  int bpp;
  bool vsync;
  bool fullScreen;
  bool loopExitFlag;
  SDL_Window* screen;
  bool appExitFlag;

  bool limitFps;
  int targetFps;

  FontRef defaultFont;

  EventHandler* handler;

  void beginFrame();
  void endFrame();
};

extern Engine engine;

}

#endif
