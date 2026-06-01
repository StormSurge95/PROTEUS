# ProteusNES\src\tests\CMakeLists.txt

if (NOT PROTEUS_BUILD_TESTS)
    return()
endif()

# SST testing
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
# create individual test for each opcode
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
        LABELS "type:sst;emulator:nes;kind:cpu"
    )
endforeach()

# Determinism testing
add_executable(NesDeterminismTests
    NesDeterminismTests.cpp
    ${NES_CORE_SOURCES}
)
# target_compile_definitions(NesDeterminismTests PRIVATE SELF_TEST) # only uncomment if testing forced-failure
target_include_directories(NesDeterminismTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${PROTEUS_SRC_DIR}/shared
)
target_link_libraries(NesDeterminismTests PRIVATE nlohmann_json::nlohmann_json)
set_target_properties(NesDeterminismTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/tests"
)
target_compile_features(NesDeterminismTests PRIVATE cxx_std_20)
# create individual test for each determinism type
set(NES_DET_TYPES cpu frame audio)
foreach(TEST_KIND IN LISTS NES_DET_TYPES)
    string(TOUPPER "${TEST_KIND}" TEST_KIND_UPPER)

    if (TEST_KIND STREQUAL "cpu")
        set(NES_TEST_ROM "${NES_DET_CPU}")
    elseif (TEST_KIND STREQUAL "frame")
        set(NES_TEST_ROM "${NES_DET_FRAME}")
    elseif (TEST_KIND STREQUAL "audio")
        set(NES_TEST_ROM "${NES_DET_AUDIO}")
    else()
        set(NES_TEST_ROM "")
    endif()

    if (NES_TEST_ROM)
        add_test(
            NAME "NES.DET.${TEST_KIND_UPPER}"
            COMMAND NesDeterminismTests "${TEST_KIND}" "${NES_TEST_ROM}"
        )
        set_tests_properties("NES.DET.${TEST_KIND_UPPER}" PROPERTIES
            LABELS "type:determinism;emulator:nes;kind:${TEST_KIND}"
        )
    else()
        message(WARNING "${TEST_NAME} disabled: ROM path not set for kind '${TEST_KIND}'")
    endif()
endforeach()

# Mapper testing
add_executable(NesMapperTests
    NesMapperTests.cpp
    ${NES_CORE_SOURCES}
)
# target_compile_definitions(NesMapperTests PRIVATE SELF_TEST); # only uncomment if testing forced-failure
target_include_directories(NesMapperTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../PAK/Mappers
    ${PROTEUS_SRC_DIR}/shared
)
target_link_libraries(NesMapperTests PRIVATE nlohmann_json::nlohmann::json)
set_target_properties(NesMapperTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/tests"
)
target_compile_features(NesMapperTests PRIVATE cxx_std_20)
set(NES_MAPPERS 0 1 2 3 4 7 9 10)
set(NES_MAPPER_SCENARIOS default prg crom cram mirror ram wrapping)
foreach(MAPPER IN LISTS NES_MAPPERS)
    foreach(SCENARIO IN LISTS NES_MAPPER_SCENARIOS)
        string(TOUPPER "${SCENARIO}" TEST_KIND)
        set(TEST_NAME "NES.MAPPER${MAPPER}.${TEST_KIND}")

        if (SCENARIO STREQUAL "default")
        elseif (SCENARIO STREQUAL "prom")
        elseif (SCENARIO STREQUAL "pram")
        elseif (SCENARIO STREQUAL "crom")
        elseif (SCENARIO STREQUAL "cram")
        elseif (SCENARIO STREQUAL "mirror")
        elseif (SCENARIO STREQUAL "wrapping")
        else()
        endif()
    endforeach()
endforeach()