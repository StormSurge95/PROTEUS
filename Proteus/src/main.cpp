#include "./frontend/app/Proteus.h"

using namespace NS_Proteus;

// TODO: switch to WINDOWS mode instead of CONSOLE
int main(int i, char** args) {
    Proteus* proteus = new Proteus();
    proteus->Init();
    proteus->Run();
    delete proteus;

    return 0;
}