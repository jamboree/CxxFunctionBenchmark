project(CxxFunctionBenchmark CXX)
cmake_minimum_required(VERSION 3.2)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
      set(CMAKE_CXX_STANDARD 14)
      message("# setting CMAKE_CXX_STANDARD to ${CMAKE_CXX_STANDARD}")
    else()
      include(highest_supp_cmake_cxx_standard.cmake)

      list_of_supp_cmake_cxx_stds(lst_supp_cxx_stds)
      message("# CXX standards supported by currently running cmake (found via command:  cmake --help-property CXX_STANDARD): ${lst_supp_cxx_stds}")
      
      highest_supp_cmake_cxx_standard(hightest_cxx_std)
      set(CMAKE_CXX_STANDARD ${hightest_cxx_std})
      message("# setting CMAKE_CXX_STANDARD to hightest supported by your compiler: ${hightest_cxx_std}")
    endif()
endif()

if(NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
    message("# setting CMAKE_CXX_EXTENSIONS to ${CMAKE_CXX_EXTENSIONS}")
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("# setting CMAKE_BUILD_TYPE to ${CMAKE_BUILD_TYPE}")
endif()

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM 64)
else()
    set(PLATFORM 32)
endif()

if(MSVC)
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

    list(APPEND BOOST_LIBRARYDIR
        ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.1
        ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.0)
  else()
    message(FATAL_ERROR "No BOOST_ROOT environment variable could be found! Please make sure it is set and the points to your Boost installation.")
  endif()
else()
  # Enable full warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
endif()

find_package(Boost 1.55 REQUIRED)

add_library(base INTERFACE)

target_include_directories(base
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR})

target_compile_features(base
  INTERFACE
    cxx_alias_templates
    cxx_auto_type
    cxx_constexpr
    cxx_decltype
    cxx_decltype_auto
    cxx_final
    cxx_lambdas
    cxx_lambda_init_captures
    cxx_generic_lambdas
    cxx_variadic_templates
    cxx_defaulted_functions
    cxx_nullptr
    cxx_trailing_return_types
    cxx_return_type_deduction)

target_link_libraries(base
  INTERFACE
    Boost::boost)

file(GLOB HEADER *.hpp *.h)

add_executable(overload
  ${CMAKE_CURRENT_SOURCE_DIR}/overload.cpp)

target_link_libraries(overload
  PUBLIC
    base)

add_executable(various
  ${HEADER}
  ${CMAKE_CURRENT_SOURCE_DIR}/various.cpp)

target_link_libraries(various
  PUBLIC
    base)
