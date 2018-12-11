# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

#clang-format off
add_subdirectory(irrxml)
add_subdirectory(base64)

if(OT_BUNDLED_SQLITE)
  add_subdirectory(sqlite-amalgamation-3240000)
endif()

if(CASH_LUCRE_EXPORT)
  ### Build lucre as library
  set(lucre-sources
    ${CMAKE_CURRENT_SOURCE_DIR}/lucre/src/bankimp.cpp
  )

  if(WIN32)
    # suppress sscanf/sprintf being unsafe warnings
    add_definitions("/wd4996")
  endif()

  add_library(lucre OBJECT
    ${lucre-sources}
  )

  set_property(TARGET lucre PROPERTY POSITION_INDEPENDENT_CODE 1)
  target_include_directories(lucre
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/
    ${OT_SSL_HEADERS}
  )
  target_compile_options(lucre PRIVATE -Wno-unused-variable)
endif(CASH_LUCRE_EXPORT)

if("${CMAKE_C_COMPILER_TARGET}" STREQUAL "")
  set(OT_HOST "")
else()
  set(OT_HOST "--host=${CMAKE_C_COMPILER_TARGET}")
endif()

if("${OT_SYSROOT}" STREQUAL "")
  set(OT_SYSROOT_BUNDLED "")
else()
  set(OT_SYSROOT_BUNDLED "--with-sysroot=${OT_SYSROOT}")
endif()

if(ANDROID)
  set(OT_ANDROID_LOG "LDFLAGS=-llog")
else()
  set(OT_ANDROID_LOG "")
endif()

if("${CMAKE_MAKE_PROGRAM}" MATCHES ".*ninja.*")
  set(OT_BUNDLED_MAKE "make")
else()
  set(OT_BUNDLED_MAKE "${CMAKE_MAKE_PROGRAM}")
endif()

if (OT_BUNDLED_SSL)
    configure_file("ssl-configure.in" "${CMAKE_CURRENT_BINARY_DIR}/libressl/configure.sh" @ONLY)
    set(OT_SSL_LIBRARIES
      "${CMAKE_CURRENT_BINARY_DIR}/libressl/src/crypto/libcrypto.a"
      "${CMAKE_CURRENT_BINARY_DIR}/libressl/src/ssl/libssl.a"
      CACHE INTERNAL "" FORCE)
    include(ExternalProject)
    ExternalProject_Add(
        libressl
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libressl
        TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/libressl/tmp
        STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/libressl/stamp
        EXCLUDE_FROM_ALL 1
        DOWNLOAD_COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/libressl/src/
        DOWNLOAD_NO_PROGRESS 1
        UPDATE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/libressl/src/autogen.sh
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libressl/src
        CONFIGURE_COMMAND sh "${CMAKE_CURRENT_BINARY_DIR}/libressl/configure.sh"
        BUILD_COMMAND"${OT_BUNDLED_MAKE}"
        BUILD_IN_SOURCE 1
        BUILD_BYPRODUCTS "${OT_SSL_LIBRARIES}"
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/libressl/install
        INSTALL_COMMAND ${OT_BUNDLED_MAKE} install
    )

    ExternalProject_Add_Step(
        libressl
        checkout
        COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/libressl checkout-index --all --force --prefix=${CMAKE_CURRENT_BINARY_DIR}/libressl/src/
        DEPENDERS update
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libressl
    )
else()
    set(OT_SSL_LIBRARIES "${OPENSSL_LIBRARIES}" CACHE INTERNAL "" FORCE)
endif()

if (TREZORCRYPTO_EXPORT)
    set(trezor-sources
        trezor-crypto/aes/aescrypt.c
        trezor-crypto/aes/aeskey.c
        trezor-crypto/aes/aes_modes.c
        trezor-crypto/aes/aestab.c
        trezor-crypto/chacha20poly1305/chacha20poly1305.c
        trezor-crypto/chacha20poly1305/chacha_merged.c
        trezor-crypto/chacha20poly1305/poly1305-donna.c
        trezor-crypto/chacha20poly1305/rfc7539.c
        trezor-crypto/ed25519-donna/curve25519-donna-32bit.c
        trezor-crypto/ed25519-donna/curve25519-donna-helpers.c
        trezor-crypto/ed25519-donna/curve25519-donna-scalarmult-base.c
        trezor-crypto/ed25519-donna/ed25519.c
        trezor-crypto/ed25519-donna/ed25519-donna-32bit-tables.c
        trezor-crypto/ed25519-donna/ed25519-donna-basepoint-table.c
        trezor-crypto/ed25519-donna/ed25519-donna-impl-base.c
        trezor-crypto/ed25519-donna/ed25519-keccak.c
        trezor-crypto/ed25519-donna/ed25519-sha3.c
        trezor-crypto/ed25519-donna/modm-donna-32bit.c
        trezor-crypto/address.c
        trezor-crypto/base32.c
        trezor-crypto/base58.c
        trezor-crypto/bignum.c
        trezor-crypto/bip32.c
        trezor-crypto/bip39.c
        trezor-crypto/blake256.c
        trezor-crypto/blake2b.c
        trezor-crypto/blake2s.c
        trezor-crypto/cash_addr.c
        trezor-crypto/curves.c
        trezor-crypto/ecdsa.c
        trezor-crypto/hasher.c
        trezor-crypto/hmac.c
        trezor-crypto/memzero.c
        trezor-crypto/nem.c
        trezor-crypto/nist256p1.c
        trezor-crypto/pbkdf2.c
        trezor-crypto/rand.c
        trezor-crypto/rc4.c
        trezor-crypto/ripemd160.c
        trezor-crypto/script.c
        trezor-crypto/secp256k1.c
        trezor-crypto/segwit_addr.c
        trezor-crypto/sha2.c
        trezor-crypto/sha3.c
    )
    set(trezor-headers
        trezor-crypto/aes/aes.h
        trezor-crypto/aes/aesopt.h
        trezor-crypto/aes/aestab.h
        trezor-crypto/aes/brg_types.h
        trezor-crypto/chacha20poly1305/chacha20poly1305.h
        trezor-crypto/chacha20poly1305/ecrypt-config.h
        trezor-crypto/chacha20poly1305/ecrypt-machine.h
        trezor-crypto/chacha20poly1305/ecrypt-portable.h
        trezor-crypto/chacha20poly1305/ecrypt-sync.h
        trezor-crypto/chacha20poly1305/poly1305-donna-32.h
        trezor-crypto/chacha20poly1305/poly1305-donna.h
        trezor-crypto/chacha20poly1305/rfc7539.h
        trezor-crypto/ed25519-donna/curve25519-donna-32bit.h
        trezor-crypto/ed25519-donna/curve25519-donna-helpers.h
        trezor-crypto/ed25519-donna/curve25519-donna-scalarmult-base.h
        trezor-crypto/ed25519-donna/ed25519-donna-32bit-tables.h
        trezor-crypto/ed25519-donna/ed25519-donna-basepoint-table.h
        trezor-crypto/ed25519-donna/ed25519-donna.h
        trezor-crypto/ed25519-donna/ed25519-donna-impl-base.h
        trezor-crypto/ed25519-donna/ed25519-donna-portable.h
        trezor-crypto/ed25519-donna/ed25519.h
        trezor-crypto/ed25519-donna/ed25519-hash-custom.h
        trezor-crypto/ed25519-donna/ed25519-hash-custom-keccak.h
        trezor-crypto/ed25519-donna/ed25519-hash-custom-sha3.h
        trezor-crypto/ed25519-donna/ed25519-keccak.h
        trezor-crypto/ed25519-donna/ed25519-sha3.h
        trezor-crypto/ed25519-donna/modm-donna-32bit.h
        trezor-crypto/address.h
        trezor-crypto/base32.h
        trezor-crypto/base58.h
        trezor-crypto/bignum.h
        trezor-crypto/bip32.h
        trezor-crypto/bip39_english.h
        trezor-crypto/bip39.h
        trezor-crypto/blake256.h
        trezor-crypto/blake2b.h
        trezor-crypto/blake2_common.h
        trezor-crypto/blake2s.h
        trezor-crypto/cash_addr.h
        trezor-crypto/check_mem.h
        trezor-crypto/curves.h
        trezor-crypto/ecdsa.h
        trezor-crypto/hasher.h
        trezor-crypto/hmac.h
        trezor-crypto/memzero.h
        trezor-crypto/nem.h
        trezor-crypto/nem_serialize.h
        trezor-crypto/nist256p1.h
        trezor-crypto/options.h
        trezor-crypto/pbkdf2.h
        trezor-crypto/rand.h
        trezor-crypto/rc4.h
        trezor-crypto/ripemd160.h
        trezor-crypto/script.h
        trezor-crypto/secp256k1.h
        trezor-crypto/segwit_addr.h
        trezor-crypto/sha2.h
        trezor-crypto/sha3.h
    )
    add_library(TrezorCrypto OBJECT ${trezor-sources} ${trezor-headers})
    set_property(TARGET TrezorCrypto PROPERTY POSITION_INDEPENDENT_CODE 1)
    set_target_properties(TrezorCrypto PROPERTIES COMPILE_FLAGS "-std=c99")
    target_compile_definitions(TrezorCrypto PRIVATE "-DRAND_PLATFORM_INDEPENDENT")
    target_include_directories(TrezorCrypto PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/trezor-crypto")
    target_include_directories(TrezorCrypto PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/trezor-crypto/aes")
    target_include_directories(TrezorCrypto PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/trezor-crypto/chacha20poly1305")
    target_include_directories(TrezorCrypto PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/trezor-crypto/ed25519-donna")
endif()

if (OT_BUNDLED_PROTOBUF)
    configure_file("protobuf-configure.in" "${CMAKE_CURRENT_BINARY_DIR}/protobuf/configure.sh" @ONLY)
    set(bundled-protobuf-path "${CMAKE_CURRENT_BINARY_DIR}/protobuf/src/src/.libs/libprotobuf-lite.a" CACHE INTERNAL "" FORCE)
    include(ExternalProject)
    ExternalProject_Add(
        protobuf
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/protobuf
        TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/protobuf/tmp
        STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/protobuf/stamp
        EXCLUDE_FROM_ALL 1
        DOWNLOAD_COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/protobuf/src/
        DOWNLOAD_NO_PROGRESS 1
        UPDATE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/protobuf/src/autogen.sh
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/protobuf/src
        CONFIGURE_COMMAND sh "${CMAKE_CURRENT_BINARY_DIR}/protobuf/configure.sh"
        BUILD_COMMAND ${OT_BUNDLED_MAKE} -j
        BUILD_IN_SOURCE 1
        BUILD_BYPRODUCTS "${bundled-protobuf-path}"
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/protobuf/install
        INSTALL_COMMAND ${OT_BUNDLED_MAKE} install
    )

    ExternalProject_Add_Step(
        protobuf
        checkout
        COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/protobuf checkout-index --all --force --prefix=${CMAKE_CURRENT_BINARY_DIR}/protobuf/src/
        DEPENDERS update
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/protobuf
    )
else()
    set(bundled-protobuf-path "" CACHE INTERNAL "" FORCE)
endif()

if(OT_BUNDLED_OPENTXS_PROTO)
  add_subdirectory(opentxs-proto)
  file(GLOB opentxs-proto-headers "${CMAKE_CURRENT_SOURCE_DIR}/opentxs-proto/generated/opentxs-proto/*.pb.h")
  file(GLOB opentxs-proto-sources "${CMAKE_CURRENT_SOURCE_DIR}/opentxs-proto/generated/opentxs-proto/*.pb.cc")
  add_library(opentxs-proto-objects OBJECT "${opentxs-proto-headers}" "${opentxs-proto-sources}")
  set_property(TARGET opentxs-proto-objects PROPERTY POSITION_INDEPENDENT_CODE 1)
  target_include_directories(opentxs-proto-objects PUBLIC ${OT_PROTOBUF_HEADERS})
  target_include_directories(verify PUBLIC ${OT_PROTOBUF_HEADERS})
  install(FILES ${opentxs-proto-headers}
          DESTINATION include/opentxs-proto
          COMPONENT dev)
endif()

if(OT_BUNDLED_SODIUM)
    configure_file("sodium-configure.in" "${CMAKE_CURRENT_BINARY_DIR}/sodium/configure.sh" @ONLY)
    set(bundled-sodium-path "${CMAKE_CURRENT_BINARY_DIR}/sodium/src/src/libsodium/.libs/libsodium.a" CACHE INTERNAL "" FORCE)
    set(SODIUM_INCLUDE_DIR "${OT_SODIUM_HEADERS}" CACHE INTERNAL "" FORCE)
    set(SODIUM_LIBRARY "${bundled-sodium-path}" CACHE INTERNAL "" FORCE)
    set(SODIUM_FOUND TRUE CACHE INTERNAL "" FORCE)
    include(ExternalProject)
    # patch is used to resolve symbol conflicts with trezor-crypto:
    #   ge25519_scalarmult -> sodium_ge25519_scalarmult
    #   ge25519_double_scalarmult_vartime -> sodium_ge25519_double_scalarmult_vartime
    ExternalProject_Add(
        sodium
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/sodium
        TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/sodium/tmp
        STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/sodium/stamp
        EXCLUDE_FROM_ALL 1
        DOWNLOAD_COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/sodium/src/
        DOWNLOAD_NO_PROGRESS 1
        UPDATE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sodium/src/autogen.sh
        PATCH_COMMAND git -C ${CMAKE_CURRENT_BINARY_DIR}/sodium/src/ apply "${CMAKE_CURRENT_SOURCE_DIR}/sodium_symbol_conflict.patch"
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/sodium/src
        CONFIGURE_COMMAND sh "${CMAKE_CURRENT_BINARY_DIR}/sodium/configure.sh"
        BUILD_COMMAND ${OT_BUNDLED_MAKE} -j
        BUILD_IN_SOURCE 1
        BUILD_BYPRODUCTS "${bundled-sodium-path}"
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/sodium/install
        INSTALL_COMMAND ${OT_BUNDLED_MAKE} install
    )

    ExternalProject_Add_Step(
        sodium
        checkout
        COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/sodium checkout-index --all --force --prefix=${CMAKE_CURRENT_BINARY_DIR}/sodium/src/
        DEPENDERS patch
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sodium
    )
else()
    set(bundled-sodium-path "" CACHE INTERNAL "" FORCE)
endif()

if(OT_BUNDLED_LIBZMQ)
    configure_file("libzmq-configure.in" "${CMAKE_CURRENT_BINARY_DIR}/libzmq/configure.sh" @ONLY)
    set(bundled-zmq-path "${CMAKE_CURRENT_BINARY_DIR}/libzmq/build/lib/libzmq.a" CACHE INTERNAL "" FORCE)
    include(ExternalProject)
    ExternalProject_Add(
        libzmq
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libzmq
        TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/libzmq/tmp
        STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/libzmq/stamp
        EXCLUDE_FROM_ALL 1
        DOWNLOAD_COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/libzmq/src/
        DOWNLOAD_NO_PROGRESS 1
        UPDATE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/libzmq/src/autogen.sh
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libzmq/src
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libzmq/build
        CONFIGURE_COMMAND sh "${CMAKE_CURRENT_BINARY_DIR}/libzmq/configure.sh"
        BUILD_COMMAND ${OT_BUNDLED_MAKE} -j
        BUILD_IN_SOURCE 0
        BUILD_BYPRODUCTS "${bundled-zmq-path}"
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/libzmq/install
        INSTALL_COMMAND ${OT_BUNDLED_MAKE} install
    )

    ExternalProject_Add_Step(
        libzmq
        checkout
        COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/libzmq checkout-index --all --force --prefix=${CMAKE_CURRENT_BINARY_DIR}/libzmq/src/
        DEPENDERS update
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libzmq
    )
    if(OT_BUNDLED_SODIUM)
      add_dependencies(libzmq sodium)
    endif()
else()
    set(bundled-zmq-path "" CACHE INTERNAL "" FORCE)
endif()

if (OT_BUNDLED_SECP256K1)
    configure_file("secp256k1-configure.in" "${CMAKE_CURRENT_BINARY_DIR}/secp256k1/configure.sh" @ONLY)
    set(bundled-secp256k1-path "${CMAKE_CURRENT_BINARY_DIR}/secp256k1/src/.libs/libsecp256k1.a" CACHE INTERNAL "" FORCE)
    include(ExternalProject)
    ExternalProject_Add(
        secp256k1
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/secp256k1
        TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/secp256k1/tmp
        STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/secp256k1/stamp
        EXCLUDE_FROM_ALL 1
        DOWNLOAD_COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/secp256k1/src/
        DOWNLOAD_NO_PROGRESS 1
        UPDATE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/secp256k1/src/autogen.sh
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/secp256k1/src
        CONFIGURE_COMMAND sh "${CMAKE_CURRENT_BINARY_DIR}/secp256k1/configure.sh"
        BUILD_COMMAND ${OT_BUNDLED_MAKE} -j
        BUILD_IN_SOURCE 1
        BUILD_BYPRODUCTS "${bundled-secp256k1-path}"
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/secp256k1/install
        INSTALL_COMMAND ${OT_BUNDLED_MAKE} install
    )

    ExternalProject_Add_Step(
        secp256k1
        checkout
        COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/secp256k1 checkout-index --all --force --prefix=${CMAKE_CURRENT_BINARY_DIR}/secp256k1/src/
        DEPENDERS update
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/secp256k1
    )
else()
    set(bundled-secp256k1-path "" CACHE INTERNAL "" FORCE)
endif()

if (OT_BUNDLED_LMDB)
    configure_file("lmdb-make.in" "${CMAKE_CURRENT_BINARY_DIR}/lmdb/make.sh" @ONLY)
    set(bundled-lmdb-path "${CMAKE_CURRENT_BINARY_DIR}/lmdb/src/libraries/liblmdb/liblmdb.a" CACHE INTERNAL "" FORCE)
    include(ExternalProject)
    ExternalProject_Add(
        lmdb
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/lmdb
        TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/lmdb/tmp
        STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/lmdb/stamp
        EXCLUDE_FROM_ALL 1
        DOWNLOAD_COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/lmdb/src/
        DOWNLOAD_NO_PROGRESS 1
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lmdb/src/libraries/liblmdb/"
        CONFIGURE_COMMAND true
        BUILD_COMMAND sh "${CMAKE_CURRENT_BINARY_DIR}/lmdb/make.sh"
        BUILD_IN_SOURCE 1
        BUILD_BYPRODUCTS "${bundled-lmdb-path}"
        INSTALL_COMMAND true
    )

    ExternalProject_Add_Step(
        lmdb
        checkout
        COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/lmdb checkout-index --all --force --prefix=${CMAKE_CURRENT_BINARY_DIR}/lmdb/src/
        DEPENDERS build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lmdb
    )
else()
    set(bundled-lmdb-path "" CACHE INTERNAL "" FORCE)
endif()

#clang-format on
