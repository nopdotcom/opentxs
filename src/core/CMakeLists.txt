# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

#// clang-format off
set(MODULE_NAME opentxs-core)

add_subdirectory(contract)
add_subdirectory(cron)
add_subdirectory(crypto)
add_subdirectory(identifier)
add_subdirectory(otprotob)
add_subdirectory(recurring)
add_subdirectory(script)
add_subdirectory(trade)
add_subdirectory(transaction)
add_subdirectory(util)

set(cxx-sources
  Account.cpp
  AccountList.cpp
  AccountVisitor.cpp
  Armored.cpp
  Cheque.cpp
  Contract.cpp
  Data.cpp
  Flag.cpp
  Identifier.cpp
  Instrument.cpp
  Item.cpp
  Ledger.cpp
  Log.cpp
  LogSource.cpp
  Message.cpp
  NumList.cpp
  Nym.cpp
  NymFile.cpp
  NymIDSource.cpp
  OTStorage.cpp
  OTTrackable.cpp
  OTTransaction.cpp
  OTTransactionType.cpp
  String.cpp
  StringXML.cpp
)

set(cxx-install-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Account.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/AccountList.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/AccountVisitor.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Armored.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Cheque.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Contract.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Data.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Flag.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Helpers.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Identifier.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Instrument.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Item.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Ledger.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Lockable.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Log.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/LogSource.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Message.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/NumList.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/Nym.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/NymFile.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/NymIDSource.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/OTStorage.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/OTStoragePB.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/OTTrackable.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/OTTransaction.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/OTTransactionType.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/String.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/StringXML.hpp"
)

set(cxx-headers
  "${cxx-install-headers}"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/UniqueQueue.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Armored.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Data.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Flag.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Identifier.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/NymFile.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/String.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/StringXML.hpp"
)

if(KEYRING_GNOME)
  include(GNUInstallDirs)
  target_include_directories( ${MODULE_NAME}
    PRIVATE
    ${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/glib-2.0/
    /usr/${CMAKE_INSTALL_LIBDIR}/glib-2.0/include/
    ${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/gnome-keyring-1/
  )
endif()

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
        DESTINATION include/opentxs/core
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})

if(OT_GENERATE_PROTO)
  add_dependencies(${MODULE_NAME} otprotob)
endif()

target_include_directories(${MODULE_NAME}
  PRIVATE
    ${OT_SODIUM_HEADERS}
    ${ZLIB_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${ProtobufIncludePath}
)

target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../deps/")
#// clang-format on
