
#include "engine/Engine.h"
using namespace engine;

#ifdef EDITOR
#include "editor/Editor.h"
using namespace engine::editor;
#endif


int main(int argc, char* argv[])
{
#ifdef EDITOR
    Editor.Run();
#else
    Engine.Run();
#endif

    return 0;
}
