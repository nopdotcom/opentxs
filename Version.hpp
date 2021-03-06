// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// clang-format off

#ifndef OPENTXS_CORE_VERSION_HPP
#define OPENTXS_CORE_VERSION_HPP

#define OPENTXS_VERSION_MAJOR @VERSION_MAJOR@
#define OPENTXS_VERSION_MINOR @VERSION_MINOR@
#define OPENTXS_VERSION_BUGFIX @VERSION_BUGFIX@
#define OPENTXS_VERSION_HASH "@VERSION_SHA1@"
#define OPENTXS_VERSION_STRING "@VERSION_STRING@"

#define OT_CASH @OT_CASH_EXPORT@
#define OT_CASH_USING_LUCRE @CASH_LUCRE_EXPORT@
#define OT_CASH_USING_MAGIC_MONEY @CASH_MM_EXPORT@
#define OT_CRYPTO_SHA2_VIA_OPENSSL @SHA2_VIA_OPENSSL_EXPORT@
#define OT_CRYPTO_SUPPORTED_ALGO_AES @AES_EXPORT@
#define OT_CRYPTO_SUPPORTED_KEY_ED25519 @ED25519_EXPORT@
#define OT_CRYPTO_SUPPORTED_KEY_HD @HD_EXPORT@
#define OT_CRYPTO_SUPPORTED_KEY_RSA @RSA_EXPORT@
#define OT_CRYPTO_SUPPORTED_KEY_SECP256K1 @SECP256K1_EXPORT@
#define OT_CRYPTO_SUPPORTED_SOURCE_BIP47 @BIP47_EXPORT@
#define OT_CRYPTO_USING_LIBBITCOIN @LIBBITCOIN_EXPORT@
#define OT_CRYPTO_USING_LIBSECP256K1 @LIBSECP256K1_EXPORT@
#define OT_CRYPTO_USING_OPENSSL @OPENSSL_EXPORT@
#define OT_CRYPTO_USING_TREZOR @TREZORCRYPTO_EXPORT@
#define OT_CRYPTO_WITH_BIP32 @BIP32_EXPORT@
#define OT_CRYPTO_WITH_BIP39 @BIP39_EXPORT@
#define OT_DHT @DHT_EXPORT@
#define OT_SCRIPT_CHAI @SCRIPT_CHAI_EXPORT@
#define OT_STORAGE_FS @FS_EXPORT@
#define OT_STORAGE_SQLITE @SQLITE_EXPORT@

#endif // OPENTXS_CORE_VERSION_HPP
// clang-format on
