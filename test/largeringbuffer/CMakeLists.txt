add_executable(test_largeringbuffer_runner
        test_largeringbuffer.cpp
        )

target_include_directories(test_largeringbuffer_runner
PRIVATE
${PROJECT_SOURCE_DIR}/test/include
${PROJECT_SOURCE_DIR}/include
)

custom_target_use_highest_warning_level(test_largeringbuffer_runner)

add_test(
        NAME test_largeringbuffer
        COMMAND test_largeringbuffer_runner
)
