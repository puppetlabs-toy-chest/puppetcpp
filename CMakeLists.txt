cmake_minimum_required(VERSION 2.8.4)
project(puppetcpp)

set(LIBPUPPET_VERSION_MAJOR 0)
set(LIBPUPPET_VERSION_MINOR 1)
set(LIBPUPPET_VERSION_PATCH 0)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "Defaulting to a release build.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(cotire)

enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror -Wno-unused-parameter")

# Set RPATH if not installing to a system library directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" INSTALL_IS_SYSTEM_DIR)
if ("${INSTALL_IS_SYSTEM_DIR}" STREQUAL "-1")
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

find_package(Boost 1.57.0 COMPONENTS regex)
set(CATCH_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/vendor/catch/include")
include_directories(${Boost_INCLUDE_DIRS})

# Add the executable for generating the static lexer
add_executable(generate_static_lexer lexer/generate_static_lexer.cc)
target_link_libraries(generate_static_lexer ${Boost_LIBRARIES})

# Add the command to generate the static lexer
add_custom_command(COMMAND generate_static_lexer WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/lexer OUTPUT ${CMAKE_SOURCE_DIR}/lexer/static_lexer.hpp DEPENDS lexer/lexer.hpp)

add_library(puppet SHARED
    lexer/lexer.cc
    lexer/static_lexer.hpp
    lexer/string_token.cc
    lexer/token_id.cc
    ast/access_expression.cc
    ast/array.cc
    ast/boolean.cc
    ast/case_expression.cc
    ast/class_definition_expression.cc
    ast/collection_expression.cc
    ast/defined_type_expression.cc
    ast/expression.cc
    ast/function_call_expression.cc
    ast/hash.cc
    ast/if_expression.cc
    ast/lambda.cc
    ast/manifest.cc
    ast/method_call_expression.cc
    ast/name.cc
    ast/node_definition_expression.cc
    ast/number.cc
    ast/parameter.cc
    ast/regex.cc
    ast/selector_expression.cc
    ast/undef.cc
    ast/resource_defaults_expression.cc
    ast/resource_expression.cc
    ast/resource_override_expression.cc
    ast/string.cc
    ast/type.cc
    ast/unless_expression.cc
    ast/variable.cc
    ast/visitors.cc
    parser/parser.cc
    utility/error_reporter.cc
)

set_target_properties(puppet PROPERTIES VERSION "${LIBPUPPET_VERSION_MAJOR}.${LIBPUPPET_VERSION_MINOR}.${LIBPUPPET_VERSION_PATCH}" COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY_UNITY")

target_link_libraries(puppet
    ${Boost_LIBRARIES}
)

install(TARGETS puppet DESTINATION lib)

cotire(puppet)

add_subdirectory(exe)
add_subdirectory(tests)
