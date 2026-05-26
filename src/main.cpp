#include "application.h"

int main(int argc, char** argv)
{
    ohtoai::Application app(argc, argv);
    return static_cast<int>(app.run());
}
