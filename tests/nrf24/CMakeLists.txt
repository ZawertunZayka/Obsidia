add_executable(nrf24_tests
    test_nrf24.cpp
    ../../firmware/radio-esp32/lib/nrf24_diag/src/Nrf24Diagnostic.cpp
)
target_include_directories(nrf24_tests PRIVATE
    ../../firmware/radio-esp32/lib/nrf24_diag/src
)
target_compile_features(nrf24_tests PRIVATE cxx_std_17)
target_compile_options(nrf24_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME nrf24 COMMAND nrf24_tests)
