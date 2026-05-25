#include "./frontend/app/Proteus.h"

#include "./backend/GBA/shared/GbaPCH.h"

using namespace NS_Proteus;

// TODO: switch to WINDOWS mode instead of CONSOLE
#if (WINSUB)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main(int i, int args[]) {
#endif
    Proteus* proteus = new Proteus();
    proteus->Init();
    #ifdef TEST_SST
    proteus->RunSST();
    #else
    proteus->Run();
    #endif
    delete proteus;

    return 0;
}