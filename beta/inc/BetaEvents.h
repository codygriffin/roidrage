#ifndef INCLUDED_BETAEVENTS_H
#define INCLUDED_BETAEVENTS_H

//------------------------------------------------------------------------------

namespace beta {

//------------------------------------------------------------------------------

struct Tick            {};
struct Render          {};
struct GlfwKey         { int key; int scancode; int action; int mods; };
struct GlfwMouseButton { int button; int action; int mods; float x; float y;};
struct GlfwMouseMove   { double x; double y; };
struct GlfwMouseScroll { double x; double y; };

//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#endif
