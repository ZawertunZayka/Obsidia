add_executable(input_service_tests
    test_input_service.cpp
    ../../firmware/main-s3/src/obsidia/input/InputService.cpp
)
target_include_directories(input_service_tests PRIVATE
    ../../firmware/main-s3/src/obsidia/input
)
target_compile_features(input_service_tests PRIVATE cxx_std_17)
target_compile_options(input_service_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME input_service COMMAND input_service_tests)
