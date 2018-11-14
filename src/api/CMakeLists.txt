# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-api)

add_subdirectory(client)
add_subdirectory(crypto)
add_subdirectory(network)
add_subdirectory(server)
add_subdirectory(storage)

set(cxx-sources
  Core.cpp
  Endpoints.cpp
  Factory.cpp
  HDSeed.cpp
  Identity.cpp
  Legacy.cpp
  Log.cpp
  Native.cpp
  Periodic.cpp
  Scheduler.cpp
  Settings.cpp
  StorageParent.cpp
  Wallet.cpp
)

file(GLOB cxx-install-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/api/*.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/../internal/api/Internal.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Core.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Endpoints.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Factory.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/HDSeed.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Identity.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Legacy.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Log.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Native.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Periodic.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Scheduler.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Settings.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/StorageParent.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Wallet.hpp
)

if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
    util/OTWindowsRegistryTools.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/api
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
target_compile_options(${MODULE_NAME} PRIVATE -Wno-undef) # simpleini
target_compile_options(${MODULE_NAME} PRIVATE -Wno-cast-align) # simpleini
set_lib_property(${MODULE_NAME})
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../deps/")
