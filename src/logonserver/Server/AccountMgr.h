/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <map>
#include <Database/Field.hpp>
#include <Threading/AEThread.h>

struct Account
{
    uint32_t AccountId;
    char* GMFlags;
    uint8_t AccountFlags;
    uint32_t Banned;
    uint8_t SrpHash[20]; // the encrypted password field, reversed
    uint8_t* SessionKey;
    std::string* UsernamePtr;
    std::string forcedLanguage;
    uint32_t Muted;

    Account()
    {
        GMFlags = NULL;
        SessionKey = NULL;
        AccountId = 0;
        AccountFlags = 0;
        Banned = 0;
        Muted = 0;
        forcedLocale = false;
        UsernamePtr = nullptr;
    }

    ~Account()
    {
        delete[] GMFlags;
        delete[] SessionKey;
    }

    void SetGMFlags(const char* flags)
    {
        delete[] GMFlags;

        size_t len = strlen(flags);
        if (len == 0 || (len == 1 && flags[0] == '0'))
        {
            // no flags
            GMFlags = NULL;
            return;
        }

        GMFlags = new char[len + 1];
        memcpy(GMFlags, flags, len);
        GMFlags[len] = 0;
    }

    void SetSessionKey(const uint8_t* key)
    {
        if (SessionKey == NULL)
            SessionKey = new uint8_t[40];
        memcpy(SessionKey, key, 40);
    }

    bool forcedLocale;

};

class AccountMgr
{
private:
    AccountMgr() = default;
    ~AccountMgr() = default;

public:
    static AccountMgr& getInstance();
    void initialize(uint32_t reloadTime);
    void finalize();

    AccountMgr(AccountMgr&&) = delete;
    AccountMgr(AccountMgr const&) = delete;
    AccountMgr& operator=(AccountMgr&&) = delete;
    AccountMgr& operator=(AccountMgr const&) = delete;

    void addAccount(Field* field);

    std::shared_ptr<Account> getAccountByName(std::string& Name);

    void updateAccount(std::shared_ptr<Account> account, Field* field);
    void reloadAccounts(bool silent);

    size_t getCount() const;

    std::map<std::string, std::shared_ptr<Account>> getAccountMap() const;

private:

    std::shared_ptr<Account> _getAccountByNameLockFree(std::string& Name);

    std::map<std::string, std::shared_ptr<Account>> _accountMap;

    std::unique_ptr<AscEmu::Threading::AEThread> m_reloadThread;
    uint32_t m_reloadTime;

protected:

    std::mutex accountMgrMutex;
};

#define sAccountMgr AccountMgr::getInstance()
