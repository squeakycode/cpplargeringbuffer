cmake_minimum_required(VERSION 3.12)

# We are building C++ Token Finder
project(cpplargeringbuffer CXX)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
    cmake_policy(SET CMP0074 NEW) # The NEW behavior for this policy is to use <PackageName>_ROOT variables.
endif()

set (CMAKE_CXX_STANDARD 11)

# Use highest warning levels
macro(custom_target_use_highest_warning_level TargetName)
if(MSVC)
  target_compile_options(${TargetName} PRIVATE /W4 /WX /permissive-)
else()
  target_compile_options(${TargetName} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
endmacro(custom_target_use_highest_warning_level)

option(CPP_TOKEN_FINDER_BUILD_TESTS "Determines whether to build tests." ON)
if(CPP_TOKEN_FINDER_BUILD_TESTS)
    message("Building tests.")
    include(CTest)
    add_subdirectory(test/largeringbuffer)
endif()

option(CPP_TOKEN_FINDER_GENERATE_DOXYGEN_DOCUMENTATION "API documentation will be generated using Doxygen if on." ON)
if(CPP_TOKEN_FINDER_GENERATE_DOXYGEN_DOCUMENTATION)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        message("Generating Doxygen documentation.")
        add_subdirectory(doc)
    else()
        message("Doxygen is not installed. Skipping Doxygen documentation generation")
    endif()
endif()

add_subdirectory(sample)
