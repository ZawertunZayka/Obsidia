add_executable(radio_service_tests
    test_radio_service.cpp
    ../../firmware/main-s3/src/obsidia/services/RadioService.cpp
    ../../firmware/common/obsidialink/src/obsidia_link.cpp
)

target_include_directories(radio_service_tests PRIVATE
    ../../firmware/main-s3/src/obsidia/services
    ../../firmware/common/obsidialink/include
)
target_compile_features(radio_service_tests PRIVATE cxx_std_17)
target_compile_options(radio_service_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME radio_service COMMAND radio_service_tests)
