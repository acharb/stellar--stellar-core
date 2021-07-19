// Copyright 2021 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "ledger/InMemoryLedgerTxn.h"
#include "ledger/LedgerTxnImpl.h"
#include "util/GlobalChecks.h"

namespace stellar
{

InMemoryLedgerTxn::InMemoryLedgerTxn(InMemoryLedgerTxnRoot& parent,
                                     Database& db)
    : LedgerTxn(parent), mDb(db)
{
}

InMemoryLedgerTxn::~InMemoryLedgerTxn()
{
}

void
InMemoryLedgerTxn::addChild(AbstractLedgerTxn& child)
{
    if (mTransaction)
    {
        throw std::runtime_error(
            "Adding child to already-open InMemoryLedgerTxn");
    }
    LedgerTxn::addChild(child);
    mTransaction = std::make_unique<soci::transaction>(mDb.getSession());
}

void
InMemoryLedgerTxn::commitChild(EntryIterator iter, LedgerTxnConsistency cons)
{
    if (!mTransaction)
    {
        throw std::runtime_error(
            "Committing child to non-open InMemoryLedgerTxn");
    }
    try
    {
        LedgerTxn::commitChild(iter, cons);
        mTransaction->commit();
        mTransaction.reset();
    }
    catch (std::exception& e)
    {
        printErrorAndAbort("fatal error during commit to InMemoryLedgerTxn: ",
                           e.what());
    }
    catch (...)
    {
        printErrorAndAbort(
            "unknown fatal error during commit to InMemoryLedgerTxn");
    }
}

void
InMemoryLedgerTxn::rollbackChild()
{
    if (!mTransaction)
    {
        throw std::runtime_error(
            "Rolling back child on non-open InMemoryLedgerTxn");
    }
    try
    {
        LedgerTxn::rollbackChild();
        mTransaction->rollback();
        mTransaction.reset();
    }
    catch (std::exception& e)
    {
        printErrorAndAbort(
            "fatal error when rolling back child of InMemoryLedgerTxn: ",
            e.what());
    }
    catch (...)
    {
        printErrorAndAbort(
            "unknown fatal error when rolling back child of InMemoryLedgerTxn");
    }
}

}
