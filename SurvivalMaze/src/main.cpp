#include <Logger.h>
#include "Game/Application.h"

int main(int argc, char* argv[])
{
    try
    {
        Logger::Init();
        Application app;
        CHECK(app.Init(GetModuleHandle(NULL)), 0, "Cannot initialize application");
        app.Run();
    }
    catch (const std::exception& e)
    {
        SHOWFATAL("Exception caught: {}", e.what());
    }
    catch (...)
    {
        SHOWFATAL("An unexpected error occured");
    }
    Logger::Close();
    return 0;
}
