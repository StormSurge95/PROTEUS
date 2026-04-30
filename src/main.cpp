#include "./frontend/Proteus.h"

#define WINSUB 0

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