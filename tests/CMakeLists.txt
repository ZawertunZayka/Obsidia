cmake_minimum_required(VERSION 3.16)
project(obsidia_host_tests LANGUAGES CXX)

enable_testing()
add_subdirectory(obsidialink)
add_subdirectory(radio_service)
add_subdirectory(fpga_service)
