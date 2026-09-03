add_executable(cc1101_tests
    test_cc1101.cpp
    ../../firmware/radio-esp32/lib/cc1101_diag/src/Cc1101Diagnostic.cpp
)
target_include_directories(cc1101_tests PRIVATE
    ../../firmware/radio-esp32/lib/cc1101_diag/src
)
target_compile_features(cc1101_tests PRIVATE cxx_std_17)
target_compile_options(cc1101_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME cc1101 COMMAND cc1101_tests)
