/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include "CommonTypes.hpp"

class SERVER_DECL ConfigFile
{
public:
    ConfigFile() = default;
    ~ConfigFile() = default;

    struct ConfigValueSetting
    {
        std::string asString;
        bool asBool = false;
        int asInt = 0;
        float asFloat = 0.0f;
    };

    bool openAndLoadConfigFile(const std::string& configFileName);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Parser
    bool parseConfigValues(std::string fileBufferString);
    void removeSpacesInString(std::string& str);
    void removeAllSpacesInString(std::string& str);
    bool isComment(std::string& lineString, bool* isInMultilineComment);
    void applySettingToStore(std::string& str, ConfigValueSetting& setting);

    uint32_t getSettingHash(const std::string& settingString);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Get functions
    ConfigValueSetting* getSavedSetting(const std::string& sectionName, const std::string& confName);

    std::string getStringDefault(const std::string& sectionName, const std::string& confName, const std::string& defaultString);
    bool getBoolDefault(const std::string& sectionName, const std::string& confName, bool defaultBool);
    int getIntDefault(const std::string& sectionName, const std::string& confName, int defaultInt);
    float getFloatDefault(const std::string& sectionName, const std::string& confName, float defaultFloat);

    bool tryGetBool(const std::string& sectionName, const std::string& keyName, bool* b);
    bool tryGetFloat(const std::string& sectionName, const std::string& keyName, float* f);
    bool tryGetInt(const std::string& sectionName, const std::string& keyName, int* i);
    bool tryGetInt(const std::string& sectionName, const std::string& keyName, uint8_t* i);
    bool tryGetInt(const std::string& sectionName, const std::string& keyName, uint32_t* i);
    bool tryGetString(const std::string& sectionName, const std::string& keyName, std::string* s);

private:
    using ConfigSection = std::unordered_map<uint32_t, ConfigValueSetting>;
    using ConfigStore = std::unordered_map<uint32_t, ConfigSection>;

    [[nodiscard]] uint32_t calculateSettingHash(std::string_view settingString) const noexcept;
    [[nodiscard]] const ConfigValueSetting* findSavedSetting(std::string_view sectionName, std::string_view confName) const noexcept;
    [[nodiscard]] ConfigValueSetting* findSavedSetting(std::string_view sectionName, std::string_view confName) noexcept;
    void logMissingSetting(const std::string& sectionName, const std::string& confName) const;

    ConfigStore m_settings;
};
