set(BSL_DIR ${CMAKE_CURRENT_LIST_DIR}/bde/groups/bsl)

include_directories(${BSL_DIR}/bslmf)
include_directories(${BSL_DIR}/bslalg)
include_directories(${BSL_DIR}/bslh)

set(SHORTLIB ON)

include_directories(${BSL_DIR}/bsls)
if (NOT SHORTLIB)
  file(GLOB bsls_files ${BSL_DIR}/bsls/*.cpp)
  list(FILTER bsls_files EXCLUDE REGEX "\\.t\\.cpp\$")
  add_library(bsls1 ${bsls_files})
else()
  add_library(bsls1 ${BSL_DIR}/bsls/bsls_assert.cpp
                    ${BSL_DIR}/bsls/bsls_log.cpp
                    ${BSL_DIR}/bsls/bsls_logseverity.cpp)
endif()
target_include_directories(bsls1 PUBLIC ${BSL_DIR}/bsls)
                
include_directories(${BSL_DIR}/bslscm)
if (NOT SHORTLIB)
  file(GLOB bslscm_files ${BSL_DIR}/bslscm/*.cpp)
  list(FILTER bslscm_files EXCLUDE REGEX "\\.t\\.cpp\$")
  add_library(bslscm1 ${bslscm_files})
else()
  add_library(bslscm1 ${BSL_DIR}/bslscm/bslscm_version.cpp)
endif()
target_include_directories(bslscm1 PUBLIC ${BSL_DIR}/bslscm)

include_directories(${BSL_DIR}/bslma)
if (NOT SHORTLIB)
  file(GLOB bslma_files ${BSL_DIR}/bslma/*.cpp)
  list(FILTER bslma_files EXCLUDE REGEX "\\.t\\.cpp\$")
  add_library(bslma1 ${bslma_files})
else()
  add_library(bslma1 ${BSL_DIR}/bslma/bslma_default.cpp
                     ${BSL_DIR}/bslma/bslma_newdeleteallocator.cpp
                     ${BSL_DIR}/bslma/bslma_allocator.cpp)
endif()
target_include_directories(bslma1 PUBLIC ${BSL_DIR}/bslma)
                 
include_directories(${BSL_DIR}/bslstl)
if (NOT SHORTLIB)
  file(GLOB bslstl_files ${BSL_DIR}/bslstl/*.cpp)
  list(FILTER bslstl_files EXCLUDE REGEX "\\.t\\.cpp\$")
  add_library(bslstl1 ${bslstl_files})
else()
  add_library(bslstl1 ${BSL_DIR}/bslstl/bslstl_function.cpp)
endif()
target_include_directories(bslstl1 PUBLIC ${BSL_DIR}/bslstl ${BSL_DIR}/bslh ${BSL_DIR}/bslmf ${BSL_DIR}/bslalg)
