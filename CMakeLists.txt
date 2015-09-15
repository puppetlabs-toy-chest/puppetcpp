cmake_minimum_required(VERSION 2.8.4)
project(puppetcpp)

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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

find_package(Boost 1.57.0 REQUIRED COMPONENTS program_options filesystem system)
find_package(Facter REQUIRED)
find_package(YAMLCPP REQUIRED)

# Display a summary of the features
include(FeatureSummary)
feature_summary(WHAT ALL)

set(CATCH_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/vendor/catch/include")
set(RAPIDJSON_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/vendor/rapidjson/include")
set(UTF8CPP_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/vendor/utf8cpp/include")

# Need to bump up the limit for MPL lists a little bit (default is 20)
# This is because we stick a lot of different types in boost::variant for runtime types
add_definitions(-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS -DBOOST_MPL_LIMIT_LIST_SIZE=30 -DBOOST_MPL_LIMIT_VECTOR_SIZE=30)

add_subdirectory(exe)
add_subdirectory(lib)
