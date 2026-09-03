add_executable(diagnostics_tests
    test_diagnostics.cpp
    ../../firmware/main-s3/src/obsidia/diagnostics/DiagnosticsModel.cpp
)
target_include_directories(diagnostics_tests PRIVATE
    ../../firmware/main-s3/src/obsidia/diagnostics
    ../../firmware/main-s3/src/obsidia/services
    ../../firmware/common/obsidialink/include
)
target_compile_features(diagnostics_tests PRIVATE cxx_std_17)
target_compile_options(diagnostics_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME diagnostics COMMAND diagnostics_tests)
