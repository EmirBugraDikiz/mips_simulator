cmake_minimum_required(VERSION 3.20...4.2.3)
project(MIPS_SIMULATOR C)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviourSanitizer" OFF)


# Core library

add_library(mips_core STATIC
    src/core/error_handling.c
    src/core/ir.c
    src/core/isa_mips.c
    src/core/line.c
    src/core/regmap.c
    src/core/symtab.c)


target_include_directories(mips_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(mips_core PRIVATE -Wall -Wextra -Wpedantic)


# Frontend library


add_library(mips_front STATIC
    src/front/lexer.c
    src/front/parser.c
    src/front/preprocess.c)


target_link_libraries(mips_front PUBLIC mips_core)


# Assembler library


add_library(mips_asm STATIC
    src/asm/pass1.c)


target_link_libraries(mips_asm PUBLIC mips_front)


# Simulator library   (not added yet)



# Application (main)


add_executable(mips_app app/main.c)

target_link_libraries(mips_app PRIVATE mips_asm)



# Sanitizers 

foreach(t mips_core mips_front mips_asm mips_app)

    if(ENABLE_ASAN)
        target_compile_options(${t} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${t} PRIVATE -fsanitize=address)
    endif()

    if(ENABLE_UBSAN)
        target_compile_options(${t} PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
        target_link_options(${t} PRIVATE -fsanitize=undefined)
    endif()
endforeach()



#TESTS

enable_testing()
add_subdirectory(tests/unit_tests)
