add_executable(fpga_service_tests
    test_fpga_service.cpp
    ../../firmware/main-s3/src/obsidia/services/FpgaService.cpp
)
target_include_directories(fpga_service_tests PRIVATE
    ../../firmware/main-s3/src/obsidia/services
)
target_compile_features(fpga_service_tests PRIVATE cxx_std_17)
target_compile_options(fpga_service_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME fpga_service COMMAND fpga_service_tests)
