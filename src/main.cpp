#include "editor.h"

int main(int argc, char **argv)
{
    auto app = Editor::Create();
    return app->run();
}
