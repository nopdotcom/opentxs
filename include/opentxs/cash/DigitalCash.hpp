// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CASH_DIGITALCASH_HPP
#define OPENTXS_CASH_DIGITALCASH_HPP

#include "opentxs/Forward.hpp"

#if OT_CASH
// WHICH DIGITAL CASH LIBRARY?
//
// Many algorithms may come available. We are currently using Lucre, by Ben
// Laurie,
// which is an implementation of Wagner, which is a variant of Chaum.
//
// We plan to have alternatives such as "Magic Money" by Pr0duct Cypher.
//
// Implementations for Chaum and Brands are circulating online. They could all
// be easily added here as options for Open-Transactions.

#if OT_CASH_USING_LUCRE
// IWYU pragma: begin_exports
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <lucre/bank.h>
#pragma GCC diagnostic pop
// IWYU pragma: end_exports
#endif

#if OT_CASH_USING_MAGIC_MONEY
#include...  // someday
#endif
#include <string>

namespace opentxs
{

#if OT_CASH_USING_LUCRE

class LucreDumper
{
    std::string m_str_dumpfile;

public:
    LucreDumper();
    ~LucreDumper();
};

#endif

#if OT_CASH_USING_MAGIC_MONEY

// Todo:  Someday...

#endif

}  // namespace opentxs

#endif  // OT_CASH
#endif
