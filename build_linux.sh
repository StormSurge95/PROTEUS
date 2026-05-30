#!/usr/bin/env bash
set -euo pipefail

CONFIG="debug"
TEST=false

for arg in "$@"; do
    case "$arg" in
        -D|--debug)     CONFIG="debug" ;;
        -R|--release)   CONFIG="release" ;;
        -T|--test)      TEST=true ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: ./build_linux.sh [-D|--debug] [-R|--release] [-T|--test]"
            exit 1
            ;;
    esac
done

if [[ "$CONFIG" == "debug" ]]; then
    configPreset=$([[ "$TEST" == true ]] && echo "ninja-debug-tests-x64" || echo "ninja-debug-x64")
    buildPreset=$([[ "$TEST" == true ]] && echo "ninja-build-debug-tests" || echo "ninja-build-debug")
    testPreset="ninja-test-debug"
else
    configPreset=$([[ "$TEST" == true ]] && echo "ninja-release-tests-x64" || echo "ninja-release-x64")
    buildPreset=$([[ "$TEST" == true ]] && echo "ninja-build-release-tests" || echo "ninja-build-release")
    testPreset="ninja-test-release"
fi

cmake --preset "$configPreset"
cmake --build --preset "$buildPreset"

if [[ "$TEST" == true ]]; then
    ctest --preset "$testPreset"
fi