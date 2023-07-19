cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# this setting needs to be put forward to project() command
include(${CMAKE_SOURCE_DIR}/cmake/toolchain-Cortex-M3.cmake)

# set project property
project(miniFOC LANGUAGES C ASM)

# set language standard and other property
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_STANDARD 17)

# this setting can export compile_commands.json file for include path searching for VScode
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

add_compile_options(
    -Wall # print warning info
    -Wextra # print extra warning info
    # -Wconversion # data conversion truncation warning
    # -Wsign-conversion # sign <-> unsigned conversion warning
)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif()

include_directories(
    ./
    Core/Inc
    Hardware/Inc
    Components/Inc
    Drivers/STM32F1xx_HAL_Driver/Inc
    Drivers/CMSIS/Device/ST/STM32F1xx/Include
    Drivers/CMSIS/Include
    Drivers/Qfplib-M3
)

add_definitions(
    -DUSE_FULL_LL_DRIVER
    -DSTM32F103xB
    -DHSE_VALUE=8000000
    -DLSE_VALUE=32768
    -DHSI_VALUE=8000000
    -DLSI_VALUE=40000
    -DVDD_VALUE=3300
    -DPREFETCH_ENABLE=1
)

# collect all files in the following directories and put them into 'SOURCES' variable
file(GLOB_RECURSE SOURCES
    "Drivers/*.*"
    "Core/*.*"
    "Hardware/*.*"
    "Components/*.*"
)
# append startup file
set(STARTUP_ASM ${CMAKE_SOURCE_DIR}/startup_stm32f103xb.s)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F103C8Tx_FLASH.ld)

add_link_options(
    -T ${LINKER_SCRIPT}
    LINKER:--print-memory-usage
    LINKER:-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
)

add_executable(${PROJECT_NAME} ${STARTUP_ASM} ${SOURCES} ${LINKER_SCRIPT})

set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX .elf
    LINK_DEPENDS ${LINKER_SCRIPT}
)

if(EXISTS ${CMAKE_OBJCOPY})
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}>
        ${CMAKE_BINARY_DIR}/$<TARGET_NAME:${PROJECT_NAME}>.hex
        COMMENT "Building $<TARGET_NAME:${PROJECT_NAME}>.hex"
    )

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}>
        ${CMAKE_BINARY_DIR}/$<TARGET_NAME:${PROJECT_NAME}>.bin
        COMMENT "Building $<TARGET_NAME:${PROJECT_NAME}>.bin"
    )

else()
    message(STATUS "'objcopy' not found: cannot generate .hex file")
endif()

if(EXISTS "${TOOLCHAIN_SIZE}")
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${TOOLCHAIN_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}>
        >${CMAKE_BINARY_DIR}/$<TARGET_NAME:${PROJECT_NAME}>.bsz
    )

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${TOOLCHAIN_SIZE} --format=sysv -x $<TARGET_FILE:${PROJECT_NAME}>
        >${CMAKE_BINARY_DIR}/$<TARGET_NAME:${PROJECT_NAME}>.ssz
    )
else()
    message(STATUS "'size' not found: cannot generate .[bs]sz files")
endif()