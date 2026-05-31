# ProteusNES\src\tests\CMakeLists.txt

if (NOT PROTEUS_BUILD_TESTS)
    return()
endif()

add_executable(NesSSTs
    NesSSTs.cpp
)

target_include_directories(NesSSTs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/.. # PROTEUS/ProteusNES/src
)

set_target_properties(NesSSTs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/tests"
)

target_compile_features(NesSSTs PRIVATE cxx_std_20)

foreach(OP RANGE 0 255)
    # format as two-digit uppercase hex
    math(EXPR HI "${OP} / 16")
    math(EXPR LO "${OP} % 16")
    string(SUBSTRING "0123456789ABCDEF" ${HI} 1 HICH)
    string(SUBSTRING "0123456789ABCDEF" ${LO} 1 LOCH)
    set(OPHEX "${HICH}${LOCH}")

    add_test(
        NAME NES.SST.OP_${OPHEX}
        COMMAND NesSSTs ${OPHEX} ${SSTROOT}
    )
endforeach()