cmake_minimum_required(VERSION 3.16)
project(obsidialink_tests LANGUAGES CXX)

add_executable(
    obsidialink_tests
    test_obsidia_link.cpp
    ../../firmware/common/obsidialink/src/obsidia_link.cpp
)
target_include_directories(obsidialink_tests PRIVATE ../../firmware/common/obsidialink/include)
target_compile_features(obsidialink_tests PRIVATE cxx_std_17)
target_compile_options(obsidialink_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)

enable_testing()
add_test(NAME obsidialink COMMAND obsidialink_tests)
