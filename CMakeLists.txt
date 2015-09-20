project(CxxFunctionBenchmark CXX)
cmake_minimum_required(VERSION 2.8)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceRuns)
include(CheckIncludeFiles)

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM 64)
else()
    set(PLATFORM 32)
endif()

if(MSVC)
  if(${MSVC_VERSION} LESS 1900)
    message(FATAL_ERROR "You are using an unsupported version of Visual Studio "
                        "which doesn't support all required C++14 features. "
                        "(Visual Studio 2015 (Version >= 1900) is required!)")

  endif()

  if(PLATFORM EQUAL 64)
    add_definitions("-D_WIN64")
  else()
    # ...
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

  set(BOOST_DEBUG ON)

  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)

  if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT $ENV{BOOST_ROOT})
    set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.0)
  else()
    message(FATAL_ERROR "No BOOST_ROOT environment variable could be found! Please make sure it is set and the points to your Boost installation.")
  endif()

else()
  # Check C++14 Compiler support.
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

  if(NOT COMPILER_SUPPORTS_CXX11)
    message(FATAL_ERROR "Your compiler has no C++11 capability!")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()

  # Enable full warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
endif()

set(Boost_ADDITIONAL_VERSIONS "1.59" "1.59.0")
find_package(Boost 1.55 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

file(GLOB HEADER *.hpp *.h)

add_executable(overload
  ${HEADER}
  ${CMAKE_CURRENT_SOURCE_DIR}/overload.cpp
)

add_executable(various
  ${HEADER}
  ${CMAKE_CURRENT_SOURCE_DIR}/various.cpp
)
