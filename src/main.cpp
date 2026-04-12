#include "./frontend/Proteus.h"

#define WINSUB 0

#if (WINSUB)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main(int i, int args[]) {
#endif
    Proteus* proteus = new Proteus(0);

    proteus->Init();
    proteus->Run();

    exit(EXIT_SUCCESS);
}