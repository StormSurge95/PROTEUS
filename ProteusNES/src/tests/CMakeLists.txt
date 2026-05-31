# ProteusNES\src\tests\CMakeLists.txt

if (NOT PROTEUS_BUILD_TESTS)
    return()
endif()

add_executable(NesSingleStateTests
    NesSingleStateTests.cpp
    ${NES_CORE_SOURCES}
)
target_compile_definitions(NesSingleStateTests PRIVATE TEST_SST)

target_include_directories(NesSingleStateTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(NesSingleStateTests PRIVATE
    nlohmann_json::nlohmann_json
)

set_target_properties(NesSingleStateTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/tests"
)

target_compile_features(NesSingleStateTests PRIVATE cxx_std_20)

foreach(OP RANGE 0 255)
    # format as two-digit uppercase hex
    math(EXPR HI "${OP} / 16")
    math(EXPR LO "${OP} % 16")
    string(SUBSTRING "0123456789ABCDEF" ${HI} 1 HICH)
    string(SUBSTRING "0123456789ABCDEF" ${LO} 1 LOCH)
    set(OPHEX ${HICH}${LOCH})
    set(TEST_NAME NES.SST.OP_${OPHEX})

    add_test(
        NAME ${TEST_NAME}
        COMMAND NesSingleStateTests ${OPHEX} ${SSTROOT}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "sst"
    )
endforeach()