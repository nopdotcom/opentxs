/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_STORAGE_STORAGEFS_HPP
#define OPENTXS_STORAGE_STORAGEFS_HPP

#include <opentxs/storage/Storage.hpp>

namespace opentxs
{

// Simple filesystem implementation of opentxs::storage
class StorageFS : public Storage
{
private:
    typedef Storage ot_super;

    friend Storage;

    std::string folder_;

    static std::string GetBucketName(const bool bucket)
    {
        return bucket ? "b" : "a";
    }

    StorageFS() = delete;
    // param is interpreted to mean a full path to the folder where keys should
    // be stored
    StorageFS(
        const std::string& param,
        const Digest& hash,
        const Random& random);
    StorageFS(const StorageFS&) = delete;
    StorageFS& operator=(const StorageFS&) = delete;

    using ot_super::Init;
    void Init(const std::string& param);
    void Purge(const std::string& path);

    void Cleanup_StorageFS();
public:
    std::string LoadRoot() override;
    bool StoreRoot(const std::string& hash) override;
    using ot_super::Load;
    bool Load(
        const std::string& key,
        std::string& value,
        const bool bucket) override;
    using ot_super::Store;
    bool Store(
        const std::string& key,
        const std::string& value,
        const bool bucket) override;
    bool EmptyBucket(const bool bucket) override;

    void Cleanup() override;
    ~StorageFS();
};

}  // namespace opentxs
#endif // OPENTXS_STORAGE_STORAGEFS_HPP
