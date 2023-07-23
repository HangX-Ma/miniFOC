# CMake cross compilation configuration, Generic is used for a bare-metal embedded system.
# [Optional]: Linux, QNX, WindowsCE, Android, etc.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1) # This can be anything we want
set(CMAKE_CROSSCOMPILING TRUE)

# use this to avoid running the linker during test compilation
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if (CMAKE_HOST_WIN32)
  set (SUFFIX .exe)
else()
  set (SUFFIX "")
endif()

# setting either of the compilers builds the absolute paths for the other tools:
#   ar, nm, objcopy, objdump, ranlib, readelf -- but not as, ld, size
find_program(CROSS_GCC_PATH "arm-none-eabi-gcc")
if (NOT CROSS_GCC_PATH)
    message(FATAL_ERROR "Cannot find ARM GCC compiler: arm-none-eabi-gcc")
endif()
get_filename_component(TOOLCHAIN ${CROSS_GCC_PATH} PATH)

set(CMAKE_C_COMPILER ${TOOLCHAIN}/arm-none-eabi-gcc${SUFFIX})
set(CMAKE_Cxx_COMPILER ${TOOLCHAIN}/arm-none-eabi-g++${SUFFIX})
set(CMAKE_ASM_COMPILER ${TOOLCHAIN}/arm-none-eabi-gcc${SUFFIX})
set(CMAKE_AR ${TOOLCHAIN}/arm-none-eabi-ar${SUFFIX}) # archive tool
# [CACHE INTERNAL = GLOBAL]
set(CMAKE_OBJCOPY ${TOOLCHAIN}/arm-none-eabi-objcopy${SUFFIX} CACHE INTERNAL "arm-none-eabi-objcopy") # .elf convertor tool: bin <-> hex
set(CMAKE_OBJDUMP ${TOOLCHAIN}/arm-none-eabi-objdump${SUFFIX} CACHE INTERNAL "arm-none-eabi-objdump") # disassembler tool
set(TOOLCHAIN_AS ${TOOLCHAIN}/arm-none-eabi-as${SUFFIX} CACHE INTERNAL "arm-none-eabi-as") # assembler
set(TOOLCHAIN_LD ${TOOLCHAIN}/arm-none-eabi-ld${SUFFIX} CACHE INTERNAL "arm-none-eabi-ld") # linker
set(TOOLCHAIN_SIZE ${TOOLCHAIN}/arm-none-eabi-size${SUFFIX} CACHE INTERNAL "arm-none-eabi-size") # a tool for viewing file sizes

#Uncomment for hardware floating point
#add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
# add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
# add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

#Uncomment for software floating point
# add_compile_options(-mfloat-abi=soft)

set(ARM_OPTIONS -mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_compile_options(
    ${ARM_OPTIONS}
     # create independent sections, whose name is consistent with
     # the function name or symbol name, for each function or symbol
    -ffunction-sections
    -fdata-sections
    -fno-common # Do not put uninitialized global variables into the common segment
    # -nostartfiles
    -nodefaultlibs
    -nostdlib
    -fmessage-length=0 # The output information wraps automatically according to the width of the console
)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# use these options to verify the linker can create an ELF file
# when not doing a static link
add_link_options(
    ${ARM_OPTIONS}
    # $<$<CONFIG:DEBUG>:--specs=rdimon.specs> # debug runtime
    $<$<CONFIG:DEBUG>:--specs=nano.specs>
    $<$<CONFIG:DEBUG>:--specs=nosys.specs>
    # undefined reference to `__errno‘ 解决办法: <https://blog.csdn.net/u013866683/article/details/110408463>
    $<$<CONFIG:DEBUG>:--specs=rdimon.specs>
    $<$<CONFIG:RELEASE>:--specs=nosys.specs> # bare metal release
    $<$<CONFIG:RELEASE>:--specs=rdimon.specs>
#    $<$<CONFIG:DEBUG>:-u_printf_float # enable printf function output float
#    $<$<CONFIG:DEBUG>:-u_scanf_float  # enable scanf function output float
    # LINKER: To pass options to the linker tool.
    # CMake uses the LINKER: prefix to indicate a linker specific directive.
    LINKER:--gc-sections # linker ld will delete unused function, reducing executable file size.
)

# This is a standard definition that basically says the toolchain
# commands (programs) are outside the project, but libraries, packages
# and include file locations are within the project folder hierarchy.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)