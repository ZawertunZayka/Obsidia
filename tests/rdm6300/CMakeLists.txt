add_executable(rdm6300_tests
    test_rdm6300.cpp
    ../../firmware/radio-esp32/lib/rdm6300/src/Rdm6300Parser.cpp
)
target_include_directories(rdm6300_tests PRIVATE
    ../../firmware/radio-esp32/lib/rdm6300/src
)
target_compile_features(rdm6300_tests PRIVATE cxx_std_17)
target_compile_options(rdm6300_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME rdm6300 COMMAND rdm6300_tests)
