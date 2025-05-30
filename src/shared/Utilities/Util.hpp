/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <algorithm>
#include <chrono>
#include <map>
#include <filesystem>
#include <locale>

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // WoW String functions

    /*! \brief Returns wow specific language string to id*/
    uint8_t getLanguagesIdFromString(const std::string& langstr);

    /*! \brief Returns wow specific language id to string*/
    std::string getLanguagesStringFromId(uint8_t id);

    /*! \brief Returns an uint32_t from a string between start/endcharacter */
    uint32_t getNumberFromStringByRange(const std::string& string, int startCharacter, int endCharacter);

    //////////////////////////////////////////////////////////////////////////////////////////
    // utf8String functions

    std::size_t max_consecutive(std::string_view name, bool case_insensitive = false, const std::locale& locale = std::locale());

    //////////////////////////////////////////////////////////////////////////////////////////
    // Time calculation/formatting

    /*! \brief Returns the current point in time */
    std::chrono::high_resolution_clock::time_point TimeNow();

    /*! \ brief Returns TimeNow() as time_t*/
    time_t getTimeNow();

    /*! \ brief Returns TimeNow() as uint32_t*/
    uint32_t getMSTime();

    /*! \brief Returns the difference between start_time and now in milliseconds */
    long long GetTimeDifferenceToNow(const std::chrono::high_resolution_clock::time_point& start_time);

    /*! \brief Returns the difference between start_time and end_time in milliseconds */
    long long GetTimeDifference(const std::chrono::high_resolution_clock::time_point& start_time, const std::chrono::high_resolution_clock::time_point& end_time);

    /*! \brief Returns the current Date Time as string */
    std::string GetCurrentDateTimeString();

    /*! \brief Returns the current Time as string */
    std::string GetCurrentTimeString();

    /*! \brief Returns Date Time as string from timestamp */
    std::string GetDateTimeStringFromTimeStamp(uint32_t timestamp);

    /*! \brief Returns years months days hours minutes seconds as string from seconds value */
    std::string GetDateStringFromSeconds(uint32_t seconds);

    /*! \brief Returns calculated time based on (second) values e.g. 5h will return 5 * 60 * 60 */
    uint32_t GetTimePeriodFromString(const char* str);

    /*! \brief Returns generated time value for client packets */
    uint32_t getGameTime();

    time_t getLocalHourTimestamp(time_t time, uint8_t hour, bool onlyAfterTime = true);

    std::string ByteArrayToHexString(uint8_t const* bytes, uint32_t arrayLength, bool reverseArray = false);


    //////////////////////////////////////////////////////////////////////////////////////////
    // C++17 filesystem dependent functions

    /*! \brief Returns map of directory file names. */
    std::map<uint32_t, std::string> getDirectoryContent(const std::string& pathName, const std::string& specialSuffix = "", bool withPath = false);

    /*! \brief Reads the file into a string based on the given path. */
    std::string readFileIntoString(std::filesystem::path path);

    /*! \brief Returns the first 8 chars of the file name as major version. */
    uint32_t readMajorVersionFromString(const std::string& fileName);

    uint32_t readMinorVersionFromString(const std::string& fileName);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    unsigned int makeIP(std::string_view _str);

    bool parseCIDRBan(uint32_t _ip, uint32_t _mask, uint32_t _maskBits);
}
