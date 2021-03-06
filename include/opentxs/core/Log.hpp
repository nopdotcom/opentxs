// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_LOG_HPP
#define OPENTXS_CORE_LOG_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/String.hpp"

#include <chrono>
#include <cstdint>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>

#if defined(unix) || defined(__unix__) || defined(__unix) ||                   \
    defined(__APPLE__) || defined(linux) || defined(__linux) ||                \
    defined(__linux__)
#define PREDEF_PLATFORM_UNIX 1
#endif

#if defined(debug) || defined(_DEBUG) || defined(DEBUG)
#define PREDEF_MODE_DEBUG 1
#endif

namespace opentxs
{

typedef std::deque<String*> dequeOfStrings;

class OTLogStream;

namespace api
{

class Settings;

}  // namespace api

#ifdef _WIN32
#ifdef OTLOG_IMPORT
#undef OTLOG_IMPORT
#define OTLOG_IMPORT __declspec(dllexport)
#else
#define OTLOG_IMPORT __declspec(dllimport)
#endif
#else
#ifndef OTLOG_IMPORT
#define OTLOG_IMPORT
#endif
#endif

OTLOG_IMPORT extern OTLogStream otErr;   // logs using OTLog::vError()
OTLOG_IMPORT extern OTLogStream otOut;   // logs using OTLog::vOutput(0)
OTLOG_IMPORT extern OTLogStream otWarn;  // logs using OTLog::vOutput(1)
OTLOG_IMPORT extern OTLogStream otInfo;  // logs using OTLog::vOutput(2)
OTLOG_IMPORT extern OTLogStream otLog3;  // logs using OTLog::vOutput(3)
OTLOG_IMPORT extern OTLogStream otLog4;  // logs using OTLog::vOutput(4)
OTLOG_IMPORT extern OTLogStream otLog5;  // logs using OTLog::vOutput(5)

class OTLogStream : public std::ostream, std::streambuf
{
private:
    int logLevel{0};
    int next{0};
    char* pBuffer{nullptr};
    std::recursive_mutex lock_;

public:
    explicit OTLogStream(int _logLevel);
    ~OTLogStream();

    virtual int overflow(int c) override;
};

class Log
{
private:
    static Log* pLogger;
    static const String m_strVersion;
    static const String m_strPathSeparator;

    const api::Settings& config_;
    std::int32_t m_nLogLevel{0};
    bool m_bInitialized{false};
    bool write_log_file_{false};
    String m_strThreadContext{""};
    String m_strLogFileName{""};
    String m_strLogFilePath{""};
    dequeOfStrings logDeque{};
    std::recursive_mutex lock_;

    /** For things that represent internal inconsistency in the code. Normally
     * should NEVER happen even with bad input from user. (Don't call this
     * directly. Use the above #defined macro instead.) */
    static Assert::fpt_Assert_sz_n_sz(logAssert);
    static bool CheckLogger(Log* pLogger);

    Log(const api::Settings& config);
    Log() = delete;
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    Log& operator=(const Log&) = delete;
    Log& operator=(Log&&) = delete;

public:
    /** now the logger checks the global config file itself for the
     * log-filename. */
    EXPORT static bool Init(
        const api::Settings& config,
        const String& strThreadContext = "",
        const std::int32_t& nLogLevel = 0);

    EXPORT static bool IsInitialized();

    EXPORT static bool Cleanup();

    // OTLog Constants.
    //

    // Compiled into OTLog:

    EXPORT static const char* Version();
    EXPORT static const String& GetVersion();

    EXPORT static const char* PathSeparator();
    EXPORT static const String& GetPathSeparator();

    // Set in constructor:

    EXPORT static const String& GetThreadContext();

    EXPORT static const char* LogFilePath();
    EXPORT static const String& GetLogFilePath();

    EXPORT static std::int32_t LogLevel();
    EXPORT static bool SetLogLevel(const std::int32_t& nLogLevel);

    // OTLog Functions:
    //

    EXPORT static bool LogToFile(const String& strOutput);

    /** We keep 1024 logs in memory, to make them available via the API. */
    EXPORT static std::int32_t GetMemlogSize();
    EXPORT static String GetMemlogAtIndex(std::int32_t nIndex);
    EXPORT static String PeekMemlogFront();
    EXPORT static String PeekMemlogBack();
    EXPORT static bool PopMemlogFront();
    EXPORT static bool PopMemlogBack();
    EXPORT static bool PushMemlogFront(const String& strLog);
    EXPORT static bool Sleep(const std::chrono::microseconds us);

    /** Output() logs normal output, which carries a verbosity level. If
     * nVerbosity of a message is 0, the message will ALWAYS log. (ALL output
     * levels are higher or equal to 0.) If nVerbosity is 1, the message will
     * run only if __CurrentLogLevel is 1 or higher. If nVerbosity if 2, the
     * message will run only if __CurrentLogLevel is 2 or higher. Etc.
     * THEREFORE: The higher the verbosity level for a message, the more verbose
     * the software must be configured in order to display that message. Default
     * verbosity level for the software is 0, and output that MUST appear on the
     * screen should be set at level 0. For output that you don't want to see as
     * often, set it up to 1. Set it up even higher for the really verbose stuff
     * (e.g. only if you really want to see EVERYTHING.) */
    EXPORT static void Output(
        std::int32_t nVerbosity,
        const char* szOutput);  // stdout
    EXPORT static void vOutput(
        std::int32_t nVerbosity,
        const char* szOutput,
        ...) ATTR_PRINTF(2, 3);

    /** This logs an error condition, which usually means bad input from the
     * user, or a file wouldn't open, or something like that. This contrasted
     * with Assert() which should NEVER actually happen. The software expects
     * bad user input from time to time. But it never expects a loaded mint to
     * have a nullptr pointer. The bad input would log with Error(), whereas the
     * nullptr pointer would log with Assert(); */
    EXPORT static void Error(const char* szError);       // stderr
    EXPORT static void vError(const char* szError, ...)  // stderr
        ATTR_PRINTF(1, 2);

    /** This method will print out errno and its associated string. Optionally
     * you can pass the location you are calling it from, which will be
     * prepended to the log. */
    EXPORT static void Errno(const char* szLocation = nullptr);  // stderr

    // String Helpers
    EXPORT static bool StringFill(
        String& out_strString,
        const char* szString,
        std::int32_t iLength,
        const char* szAppend = nullptr);
};

}  // namespace opentxs

#endif
