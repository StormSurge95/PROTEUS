#include "./frontend/app/Proteus.h"

using namespace NS_Proteus;

// TODO: switch to WINDOWS mode instead of CONSOLE
int main(int i, char** args) {
    printf("Creating Proteus instance...\n");
    Proteus* proteus = new Proteus();
    printf("Proteus instance created; initializing Proteus...\n");
    proteus->Init();
    printf("Proteus Initialized; running Proteus application...\n");
    proteus->Run();
    delete proteus;

    return 0;
}