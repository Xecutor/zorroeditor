#include "Clipboard.hpp"
#include "SysHeaders.hpp"

namespace glider{

void copyToClipboard(const char* utf8text)
{
  SDL_SetClipboardText(utf8text);
}
std::string pasteFromClipboard()
{
  char* txt=SDL_GetClipboardText();
  std::string rv=txt?txt:"";
  SDL_free(txt);
  return rv;
}

}
