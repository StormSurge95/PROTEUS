#include "./frontend/Proteus.h"
#include "./backend/NES/NES.h"

using namespace NS_Proteus;

// TODO: switch to WINDOWS mode instead of CONSOLE
#if (WINSUB)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main(int i, int args[]) {
#endif
    Proteus* proteus = new Proteus();

    proteus->Init();
    proteus->Run();

    delete proteus;

    exit(EXIT_SUCCESS);

    return 0;
}