add_executable(mips_tests
    run.c
    test_lexer.c
    test_parser.c
    test_pass1.c
    test_preprocess.c)


target_link_libraries(mips_tests PRIVATE mips_asm)

target_include_directories(mips_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(mips_tests PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME mips_unit_tests COMMAND mips_tests)