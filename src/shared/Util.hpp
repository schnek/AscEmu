/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>


namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // String functions

    /*! \brief Manipulates the string to lowercase */
    void StringToLowerCase(std::string& str);

    /*! \brief Manipulates the string to uppercase */
    void StringToUpperCase(std::string& str);

    /*! \brief Capitalize word (uppercase first char, lowercase rest) */
    void CapitalizeString(std::string& str);

    /*! \brief Seperates string by seperator (one char) returns string vecotr */
    std::vector<std::string> SplitStringBySeperator(const std::string& str_src, const std::string& str_sep);

    /*! \brief Returns true if string x is in sttrin y */
    bool findXinYString(std::string& x, std::string& y);

    /*! \brief Returns wow specific language string to id*/
    uint32_t getLanguagesIdFromString(std::string langstr);

    /*! \brief Returns an uint32_t from a string between start/endcharacter */
    uint32_t getNumberFromStringByRange(std::string string, int startCharacter, int endCharacter);

    //////////////////////////////////////////////////////////////////////////////////////////
    // WString functions
    bool Utf8toWStr(std::string utf8str, std::wstring& wstr);
    bool WStrToUtf8(std::wstring wstr, std::string& utf8str);

    size_t Utf8length(std::string& utf8str);

    inline wchar_t WCharToUpper(wchar_t wchar)
    {
        if (wchar >= L'a' && wchar <= L'z')                      // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
            return wchar_t(uint16_t(wchar)-0x0020);
        if (wchar == 0x00DF)                                     // LATIN SMALL LETTER SHARP S
            return wchar_t(0x1E9E);
        if (wchar >= 0x00E0 && wchar <= 0x00F6)                  // LATIN SMALL LETTER A WITH GRAVE - LATIN SMALL LETTER O WITH DIAERESIS
            return wchar_t(uint16_t(wchar)-0x0020);
        if (wchar >= 0x00F8 && wchar <= 0x00FE)                  // LATIN SMALL LETTER O WITH STROKE - LATIN SMALL LETTER THORN
            return wchar_t(uint16_t(wchar)-0x0020);
        if (wchar >= 0x0101 && wchar <= 0x012F)                  // LATIN SMALL LETTER A WITH MACRON - LATIN SMALL LETTER I WITH OGONEK (only %2=1)
        {
            if (wchar % 2 == 1)
                return wchar_t(uint16_t(wchar)-0x0001);
        }
        if (wchar >= 0x0430 && wchar <= 0x044F)                  // CYRILLIC SMALL LETTER A - CYRILLIC SMALL LETTER YA
            return wchar_t(uint16_t(wchar)-0x0020);
        if (wchar == 0x0401 || wchar == 0x0451)                  // CYRILLIC CAPITAL LETTER IO, CYRILLIC SMALL LETTER IO
            return wchar_t(0x0401);

         return wchar;
    }

    inline wchar_t WCharToLower(wchar_t wchar)
    {
        if (wchar >= L'A' && wchar <= L'Z')                      // LATIN CAPITAL LETTER A - LATIN CAPITAL LETTER Z
            return wchar_t(uint16_t(wchar)+0x0020);
        if (wchar >= 0x00C0 && wchar <= 0x00D6)                  // LATIN CAPITAL LETTER A WITH GRAVE - LATIN CAPITAL LETTER O WITH DIAERESIS
            return wchar_t(uint16_t(wchar)+0x0020);
        if (wchar >= 0x00D8 && wchar <= 0x00DE)                  // LATIN CAPITAL LETTER O WITH STROKE - LATIN CAPITAL LETTER THORN
            return wchar_t(uint16_t(wchar)+0x0020);
        if (wchar >= 0x0100 && wchar <= 0x012E)                  // LATIN CAPITAL LETTER A WITH MACRON - LATIN CAPITAL LETTER I WITH OGONEK (only %2=0)
        {
            if (wchar % 2 == 0)
                return wchar_t(uint16_t(wchar)+0x0001);
        }
        if (wchar == 0x1E9E)                                     // LATIN CAPITAL LETTER SHARP S
            return wchar_t(0x00DF);
        if (wchar == 0x0401)                                     // CYRILLIC CAPITAL LETTER IO
            return wchar_t(0x0451);
        if (wchar >= 0x0410 && wchar <= 0x042F)                  // CYRILLIC CAPITAL LETTER A - CYRILLIC CAPITAL LETTER YA
            return wchar_t(uint16_t(wchar)+0x0020);

         return wchar;
    }

    inline bool isBasicLatinCharacter(wchar_t wchar)
    {
        if(wchar >= L'a' && wchar <= L'z')                       // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
            return true;
        if(wchar >= L'A' && wchar <= L'Z')                       // LATIN CAPITAL LETTER A - LATIN CAPITAL LETTER Z
            return true;
        return false;
    }

    inline bool isExtendedLatinCharacter(wchar_t wchar)
    {
        if(isBasicLatinCharacter(wchar))
            return true;
        if(wchar >= 0x00C0 && wchar <= 0x00D6)                   // LATIN CAPITAL LETTER A WITH GRAVE - LATIN CAPITAL LETTER O WITH DIAERESIS
            return true;
        if(wchar >= 0x00D8 && wchar <= 0x00DF)                   // LATIN CAPITAL LETTER O WITH STROKE - LATIN CAPITAL LETTER THORN
            return true;
        if(wchar == 0x00DF)                                      // LATIN SMALL LETTER SHARP S
            return true;
        if(wchar >= 0x00E0 && wchar <= 0x00F6)                   // LATIN SMALL LETTER A WITH GRAVE - LATIN SMALL LETTER O WITH DIAERESIS
            return true;
        if(wchar >= 0x00F8 && wchar <= 0x00FE)                   // LATIN SMALL LETTER O WITH STROKE - LATIN SMALL LETTER THORN
            return true;
        if(wchar >= 0x0100 && wchar <= 0x012F)                   // LATIN CAPITAL LETTER A WITH MACRON - LATIN SMALL LETTER I WITH OGONEK
            return true;
        if(wchar == 0x1E9E)                                      // LATIN CAPITAL LETTER SHARP S
            return true;
        return false;
    }

    inline bool isCyrillicCharacter(wchar_t wchar)
    {
        if(wchar >= 0x0410 && wchar <= 0x044F)                   // CYRILLIC CAPITAL LETTER A - CYRILLIC SMALL LETTER YA
            return true;
        return false;
    }

    inline bool isEastAsianCharacter(wchar_t wchar)
    {
        if(wchar >= 0x1100 && wchar <= 0x11F9)                   // Hangul Jamo
            return true;
        if(wchar >= 0x3041 && wchar <= 0x30FF)                   // Hiragana + Katakana
            return true;
        if(wchar >= 0x3131 && wchar <= 0x318E)                   // Hangul Compatibility Jamo
            return true;
        if(wchar >= 0x31F0 && wchar <= 0x31FF)                   // Katakana Phonetic Ext.
            return true;
        if(wchar >= 0x3400 && wchar <= 0x4DB5)                   // CJK Ideographs Ext. A 
            return true;
        if(wchar >= 0x4E00 && wchar <= 0x9FC3)                   // Unified CJK Ideographs
            return true;
        if(wchar >= 0xAC00 && wchar <= 0xD7A3)                   // Hangul Syllables
            return true;
        if(wchar >= 0xFF01 && wchar <= 0xFFEE)                   // Halfwidth forms
            return true;
        return false;
    }

    inline bool isNumericOrSpace(wchar_t wchar)
    {
        return (wchar >= L'0' && wchar <=L'9' || wchar == L' ');
    }

    inline bool isBasicLatinString(std::wstring wstr, bool numericOrSpace)
    {
        for(size_t i = 0; i < wstr.size(); ++i)
            if(!isBasicLatinCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
                return false;
        return true;
    }

    inline bool isExtendedLatinString(std::wstring wstr, bool numericOrSpace)
    {
        for(size_t i = 0; i < wstr.size(); ++i)
            if(!isExtendedLatinCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
                return false;
        return true;
    }

    inline bool isCyrillicString(std::wstring wstr, bool numericOrSpace)
    {
        for(size_t i = 0; i < wstr.size(); ++i)
            if(!isCyrillicCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
                return false;
        return true;
    }

    inline bool isEastAsianString(std::wstring wstr, bool numericOrSpace)
    {
        for(size_t i = 0; i < wstr.size(); ++i)
            if(!isEastAsianCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
                return false;
        return true;
    }

    inline void strToUpper(std::string& str)
    {
        std::transform( str.begin(), str.end(), str.begin(), ::toupper );
    }

    inline void WStrToUpper(std::wstring& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), WCharToUpper);
    }

    inline void WStrToLower(std::wstring& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), WCharToLower);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Time calculation/formatting

    /*! \brief Returns the current point in time */
    std::chrono::high_resolution_clock::time_point TimeNow();

    /*! \ brief Returns TimeNow() as uint32_t*/
    uint32_t getMSTime();

    /*! \brief Returns the difference between start_time and now in milliseconds */
    long long GetTimeDifferenceToNow(std::chrono::high_resolution_clock::time_point start_time);

    /*! \brief Returns the difference between start_time and end_time in milliseconds */
    long long GetTimeDifference(std::chrono::high_resolution_clock::time_point start_time, std::chrono::high_resolution_clock::time_point end_time);

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

    std::string ByteArrayToHexString(uint8_t const* bytes, uint32_t arrayLength, bool reverseArray = false);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Random number helper functions

    int getRandomInt(int end);
    int getRandomInt(int start, int end);

    uint32_t getRandomUInt(uint32_t end);
    uint32_t getRandomUInt(uint32_t start, uint32_t end);

    float getRandomFloat(float end);
    float getRandomFloat(float start, float end);


    //////////////////////////////////////////////////////////////////////////////////////////
    // C++17 filesystem dependent functions

    /*! \brief Returns map of directory file names. */
    std::map<uint32_t, std::string> getDirectoryContent(std::string pathName, std::string specialSuffix = "", bool withPath = false);

    /*! \brief Reads the file into a string based on the given path. */
    std::string readFileIntoString(fs::path path);

    /*! \brief Returns the first 8 chars of the file name as major version. */
    uint32_t readMajorVersionFromString(std::string fileName);

    uint32_t readMinorVersionFromString(std::string fileName);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Benchmark
    class BenchmarkTime
    {
    public:
        BenchmarkTime()
        {
            m_startTime = std::chrono::high_resolution_clock::now();
        }

        ~BenchmarkTime()
        {
            Stop();
        }

        void Stop()
        {
            auto endTime = std::chrono::high_resolution_clock::now();

            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTime).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

            auto duration = end - start;
            double ms = duration * 0.001;

            std::cout << "BenchmarkTime: " << duration << " microseconds (" << ms << "ms)\n";
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
    };
}

struct SmallTimeTracker
{
    int32_t mExpireTime;

    public:

        SmallTimeTracker(uint32_t expired = 0) : mExpireTime(expired) {}

        void updateTimer(int32_t diffTime) { mExpireTime -= diffTime; }
        void resetInterval(uint32_t intervalTime) { mExpireTime = intervalTime; }

        int32_t getExpireTime() const { return mExpireTime; }
        bool isTimePassed() const { return mExpireTime <= 0; }
};
