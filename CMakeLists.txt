project(CxxFunctionBenchmark CXX)
cmake_minimum_required(VERSION 3.2)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

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

option(CLUGSTON "Include Clugston's FastDelegate" OFF)
if (CLUGSTON)
  include_directories(SYSTEM clugston_styled) # ignore warnings from FastDelegate.h and FastFunc.hpp
  add_definitions(-DADD_CLUGSTON)
endif()

option(SSVU "Include ssvu::FastFunc" OFF)
if (SSVU)
  include_directories(SYSTEM clugston_styled)
  add_definitions(-DADD_SSVU)
endif()

option(GNR "Include gnr::forwarder" OFF)
if (GNR)
  add_definitions(-DADD_GNR)
endif()

option(FOLLY "Include folly::Function" OFF)
if (FOLLY)
  add_definitions(-DADD_FOLLY)
endif()

option(BDE "Include Bloomberg BDE" OFF)
if (BDE)
  find_package(Git)
  execute_process (COMMAND ${GIT_EXECUTABLE} submodule update --recursive --init
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  add_definitions(-DADD_BDE)
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

if (BDE)
  add_subdirectory(bde)
endif()

add_executable(overload
  ${CMAKE_CURRENT_SOURCE_DIR}/overload.cpp)

target_link_libraries(overload
  PUBLIC
    base)

add_executable(various
  ${CMAKE_CURRENT_SOURCE_DIR}/various.cpp)

target_link_libraries(various
  PUBLIC
  base)

if (BDE)
target_link_libraries(various
  PUBLIC
  bsls1 bslma1 bslscm1 bslstl1)
endif()


