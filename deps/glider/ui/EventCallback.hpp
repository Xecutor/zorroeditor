#ifndef __GLIDER_UI_EVENTCALLBACK_HPP__
#define __GLIDER_UI_EVENTCALLBACK_HPP__

#include <functional>

namespace glider{
namespace ui{

template <class ARG>
using EventCallback=std::function<void(ARG)>;

using EventCallback0=std::function<void()>;

#define MKCALLBACK(name) std::bind(&std::remove_reference<decltype(*this)>::type::name,glider::ReferenceTmpl<std::remove_reference<decltype(*this)>::type>(this), std::placeholders::_1)
#define MKCALLBACK0(name) std::bind(&std::remove_reference<decltype(*this)>::type::name,glider::ReferenceTmpl<std::remove_reference<decltype(*this)>::type>(this))

}
}

#endif
