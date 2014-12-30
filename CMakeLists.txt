cmake_minimum_required(VERSION 2.8.4)
project(puppetcpp)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "Defaulting to a release build.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror -Wno-unused-parameter")

find_package(Boost 1.57.0 COMPONENTS regex)
include_directories(${Boost_INCLUDE_DIRS})

# Add the executable for generating the static lexer
add_executable(generate_static_lexer lexer/generate_static_lexer.cc)
target_link_libraries(generate_static_lexer ${Boost_LIBRARIES})

# Add the command to generate the static lexer
add_custom_command(COMMAND generate_static_lexer WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/lexer OUTPUT ${CMAKE_SOURCE_DIR}/lexer/static_lexer.hpp DEPENDS lexer/lexer.hpp)

# Add the executable that represents (eventually) the compiler
add_executable(puppetcpp
    main.cc
    lexer/lexer.cc
    lexer/static_lexer.hpp
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
    ast/manifest.cc
    ast/type.cc
    ast/unless_expression.cc
    ast/variable.cc
    ast/visitors.cc
)
target_link_libraries(puppetcpp ${Boost_LIBRARIES})
