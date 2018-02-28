#ifndef __GLIDER_CORE_CLIPBOARD_HPP__
#define __GLIDER_CORE_CLIPBOARD_HPP__

#include <string>

namespace glider{

void copyToClipboard(const char* utf8text);
std::string pasteFromClipboard();

}

#endif
