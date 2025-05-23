/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/EventableObject.h"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Objects/Units/Creatures/VehicleDefines.hpp"
#include "Server/Script/SimpleEventScript.hpp"
#include <string>
#include <atomic>

#if VERSION_STRING >= WotLK
    #include "AchievementMgr.h"
#endif

namespace WDB::Structures
{
#if VERSION_STRING >= WotLK
    struct DungeonEncounterEntry;
#endif
    struct SummonPropertiesEntry;
}

class Object;
class Corpse;
class Charter;
class ArenaTeam;
class SpellInfo;
class Pet;
class WorldMap;
class GameObject;
class Unit;

struct WorldState
{
    uint32_t field = 0;
    uint32_t value = 0;
};
typedef std::multimap<uint32_t, WorldState> WorldStateMap;

enum EncounterCreditType : uint8_t
{
    ENCOUNTER_CREDIT_KILL_CREATURE  = 0,
    ENCOUNTER_CREDIT_CAST_SPELL     = 1
};

#if VERSION_STRING >= WotLK
struct DungeonEncounter
{
    DungeonEncounter(WDB::Structures::DungeonEncounterEntry const* _dbcEntry, EncounterCreditType _creditType, uint32_t _creditEntry, uint32_t _lastEncounterDungeon)
        : dbcEntry(_dbcEntry), creditType(_creditType), creditEntry(_creditEntry), lastEncounterDungeon(_lastEncounterDungeon) { }

    WDB::Structures::DungeonEncounterEntry const* dbcEntry;
    EncounterCreditType creditType;
    uint32_t creditEntry;
    uint32_t lastEncounterDungeon;
};
#else
struct DungeonEncounter
{
    DungeonEncounter(EncounterCreditType _creditType, uint32_t _creditEntry)
        : creditEntry(_creditEntry), creditType(_creditType) { }

    uint32_t creditEntry;
    EncounterCreditType creditType;
};
#endif

typedef std::list<std::shared_ptr<DungeonEncounter>> DungeonEncounterList;

//it seems trainerspells should be part of trainer files ;)
struct TrainerSpell
{
    TrainerSpell() : castSpell(nullptr), castRealSpell(nullptr), learnSpell(nullptr), deleteSpell(0),
        requiredLevel(0), requiredSkillLine(0), requiredSkillLineValue(0), isPrimaryProfession(false),
        cost(0), isStatic(0)
    {
        for (uint8_t i = 0; i < 3; ++i)
            requiredSpell[i] = 0;
    }

    // This spell is casted on player
    SpellInfo const* castSpell;
    // The taught spell from castSpell
    SpellInfo const* castRealSpell;
    // This spell is added to player
    SpellInfo const* learnSpell;
    uint32_t deleteSpell;
    uint32_t requiredLevel;
    uint32_t requiredSpell[3];
    uint16_t requiredSkillLine;
    uint32_t requiredSkillLineValue;
    bool isPrimaryProfession;
    uint32_t cost;
    uint32_t isStatic;

    static uint8_t getMaxRequiredSpellCount()
    {
#if VERSION_STRING < Cata
        return 3;
#else
        return 2;
#endif
    }
};

// oh a trainer look it is here and not in Unit/Creature/whatever file ;)
struct Trainer
{
    uint32_t SpellCount;
    std::string UIMessage;
    uint16_t RequiredSkill;
    uint32_t RequiredSkillLine;
    uint32_t RequiredClass;
    uint32_t RequiredRace;
    uint32_t RequiredRepFaction;
    uint32_t RequiredRepValue;
    uint32_t TrainerType;
    uint32_t Can_Train_Gossip_TextId;
    uint32_t Cannot_Train_GossipTextId;
    uint32_t spellset_id;
    uint32_t can_train_max_level;
    uint32_t can_train_min_skill_value;
    uint32_t can_train_max_skill_value;
};

// isn't it part of player info? hmmmmm....
struct LevelInfo
{
    uint32_t HP;
    uint32_t Mana;
    uint32_t Stat[5];
};

//player too?!?
struct ReputationMod
{
    uint32_t faction[2];
    int32_t value;
    uint32_t replimit;
};

// reputation/instance
struct InstanceReputationMod
{
    uint32_t mapid;
    uint32_t mob_rep_reward;
    uint32_t mob_rep_limit;
    uint32_t boss_rep_reward;
    uint32_t boss_rep_limit;
    uint32_t faction[2];
};

// reputation/instance
struct ReputationModifier
{
    uint32_t entry;
    std::vector<std::shared_ptr<ReputationMod>> mods;
};

// reputation/instance
struct InstanceReputationModifier
{
    uint32_t mapid;
    std::vector<std::shared_ptr<InstanceReputationMod>> mods;
};


class SERVER_DECL ObjectMgr : public EventableObject
{
    ObjectMgr() = default;
    ~ObjectMgr() = default;

public:
    static ObjectMgr& getInstance();
    void initialize();
    void finalize();

    ObjectMgr(ObjectMgr&&) = delete;
    ObjectMgr(ObjectMgr const&) = delete;
    ObjectMgr& operator=(ObjectMgr&&) = delete;
    ObjectMgr& operator=(ObjectMgr const&) = delete;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Arena Team
    void loadArenaTeams();

    void addArenaTeam(std::shared_ptr<ArenaTeam> _arenaTeam);
    void removeArenaTeam(std::shared_ptr<ArenaTeam> _arenaTeam);

    std::shared_ptr<ArenaTeam> getArenaTeamByName(std::string& _name, uint32_t _type);
    std::shared_ptr<ArenaTeam> getArenaTeamById(uint32_t _id);
    std::shared_ptr<ArenaTeam> getArenaTeamByGuid(uint32_t _guid, uint32_t _type);

    void updateArenaTeamRankings();
    void updateArenaTeamWeekly();
    void resetArenaTeamRatings();

private:
    std::unordered_map<uint32_t, std::shared_ptr<ArenaTeam>> m_arenaTeams;
    std::unordered_map<uint32_t, std::shared_ptr<ArenaTeam>> m_arenaTeamMap[3];
    std::mutex m_arenaTeamLock;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Charter
public:
    void loadCharters();
    void removeCharter(const std::shared_ptr<Charter>&);
    std::shared_ptr<Charter> createCharter(uint32_t _leaderGuid, CharterTypes _type);

    std::shared_ptr<Charter> getCharterByName(const std::string& _charterName, CharterTypes _type);
    std::shared_ptr<Charter> getCharter(uint32_t _charterId, CharterTypes _type);
    std::shared_ptr<Charter> getCharterByGuid(uint64_t _playerguid, CharterTypes _type);
    std::shared_ptr<Charter> getCharterByItemGuid(uint64_t _guid);

private:
    std::unordered_map<uint32_t, std::shared_ptr<Charter>> m_charters[NUM_CHARTER_TYPES];
    std::mutex m_charterLock;

    //////////////////////////////////////////////////////////////////////////////////////////
    // CachedCharacterInfo
public:
    void loadCharacters();
    void addCachedCharacterInfo(const std::shared_ptr<CachedCharacterInfo>& _characterInfo);
    std::shared_ptr<CachedCharacterInfo> getCachedCharacterInfo(uint32_t _playerGuid);
    std::shared_ptr<CachedCharacterInfo> getCachedCharacterInfoByName(std::string _playerName);
    void updateCachedCharacterInfoName(const std::shared_ptr<CachedCharacterInfo>& _characterInfo, const std::string& _newName);
    void deleteCachedCharacterInfo(uint32_t _playerGuid);

private:
    std::unordered_map<uint32_t, std::shared_ptr<CachedCharacterInfo>> m_cachedCharacterInfo;
    std::mutex m_cachedCharacterLock;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Corpse
public:
    void loadCorpsesForInstance(WorldMap* _worldMap) const;
    std::shared_ptr<Corpse> loadCorpseByGuid(uint32_t _corpseGuid) const;
    std::shared_ptr<Corpse> createCorpse();

    void addCorpse(const std::shared_ptr<Corpse>&);
    void removeCorpse(const std::shared_ptr<Corpse>&);

    std::shared_ptr<Corpse> getCorpseByGuid(uint32_t _corpseGuid);
    std::shared_ptr<Corpse> getCorpseByOwner(uint32_t _playerGuid);

    void unloadCorpseCollector();
    void addCorpseDespawnTime(const std::shared_ptr<Corpse>& _corpse);
    void delinkCorpseForPlayer(const Player* _player);

private:
    std::unordered_map<uint32_t, std::shared_ptr<Corpse>> m_corpses;
    std::mutex m_corpseLock;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Vendors
public:
    void loadVendors();

    std::shared_ptr<std::vector<CreatureItem>> getVendorList(uint32_t _entry);
    void setVendorList(uint32_t _entry, std::shared_ptr<std::vector<CreatureItem>> _list);

private:
    std::unordered_map<uint32_t, std::shared_ptr<std::vector<CreatureItem>>> m_vendors;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Achievement - global achievement information
#if VERSION_STRING > TBC
public:
    void loadAchievementCriteriaList();
    void loadAchievementRewards();
    void loadCompletedAchievements();

    AchievementReward const* getAchievementReward(uint32_t _entry, uint8_t _gender);
    AchievementCriteriaEntryList const& getAchievementCriteriaByType(AchievementCriteriaTypes _type);

    void addCompletedAchievement(uint32_t _achievementId);
    bool isInCompletedAchievements(uint32_t _achievementId);

private:
    AchievementRewardsMap m_achievementRewards;
    AchievementCriteriaEntryList m_AchievementCriteriasByType[ACHIEVEMENT_CRITERIA_TYPE_TOTAL];
#if VERSION_STRING > WotLK
    AchievementCriteriaEntryList m_GuildAchievementCriteriasByType[ACHIEVEMENT_CRITERIA_TYPE_TOTAL];
#endif

    std::set<uint32_t> m_allCompletedAchievements;
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Reputation Mods
public:
    typedef std::unordered_map<uint32_t, std::shared_ptr<ReputationModifier>> ReputationModMap;
    void loadReputationModifiers();
    void loadReputationModifierTable(const char* _tableName, ReputationModMap& _reputationModMap);
    void loadInstanceReputationModifiers();

    std::shared_ptr<ReputationModifier> getReputationModifier(uint32_t _entry, uint32_t _factionId);
    bool handleInstanceReputationModifiers(Player* _player, Unit* _unitVictim);

private:
    ReputationModMap m_reputationFaction;
    ReputationModMap m_reputationCreature;
    std::unordered_map<uint32_t, std::shared_ptr<InstanceReputationModifier>> m_reputationInstance;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Group
public:
    void loadGroups();
    void loadGroupInstances();

    uint32_t generateGroupId();

    void addGroup(std::shared_ptr<Group> _group);
    void removeGroup(uint32_t _groupId);

    std::shared_ptr<Group> getGroupByLeader(Player* _player);
    std::shared_ptr<Group> getGroupById(uint32_t _id);

private:
    std::mutex m_groupLock;
    std::unordered_map<uint32_t, std::shared_ptr<Group>> m_groups;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Player
public:
    Player* createPlayer(uint8_t _class);
    Player* createPlayerByGuid(uint8_t _class, uint32_t _guid);

    Player* getPlayer(const char* _name, bool _caseSensitive = true);
    Player* getPlayer(uint32_t guid);

    void addPlayer(Player* _player);
    void removePlayer(Player* _player);

    void resetDailies();

    std::unordered_map<uint32_t, Player*> getPlayerStorage();

    std::mutex m_playerLock;

private:
    std::unordered_map<uint32_t, Player*> m_players;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Vehicle
#ifdef FT_VEHICLES
public:
    void loadVehicleAccessories();
    VehicleAccessoryList const* getVehicleAccessories(uint32_t _entry);

    void loadVehicleSeatAddon();
    VehicleSeatAddon const* getVehicleSeatAddon(uint32_t _seatId) const;

private:
    VehicleAccessoryContainer m_vehicleAccessoryStore;
    VehicleSeatAddonContainer m_vehicleSeatAddonStore;
#endif
    //////////////////////////////////////////////////////////////////////////////////////////
    // EventScripts
public:
    void loadEventScripts();
    EventScriptBounds getEventScripts(uint32_t _eventId) const;
    SpellEffectMapBounds getSpellEffectBounds(uint32_t _data1) const;
    bool checkForScripts(Player* _player, uint32_t _eventId);
    bool checkForDummySpellScripts(Player* _player, uint32_t _data1);
    void eventScriptsUpdate(Player* _player, uint32_t _nextEvent);

private:
    EventScriptMaps m_eventScriptMaps;
    SpellEffectMaps m_spellEffectMaps;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
public:
    void generateDatabaseGossipMenu(Object* _object, uint32_t _gossipMenuId, Player* _player, uint32_t _forcedTextId = 0);
    void generateDatabaseGossipOptionAndSubMenu(Object* _object, Player* _player, uint32_t _gossipItemId, uint32_t _gossipMenuId);

    void loadTrainerSpellSets();
    std::shared_ptr<std::vector<TrainerSpell>> getTrainerSpellSetById(uint32_t _id);

    void loadTrainers();
    std::shared_ptr<Trainer> getTrainer(uint32_t _entry);

    // Preload CreatureDisplayInfoStore and CreatureModelDataStore to avoid DBC lookup calls
    void loadCreatureDisplayInfo();
    CreatureDisplayInfoData const* getCreatureDisplayInfoData(uint32_t _displayId) const;

    GameObject* createGameObjectByGuid(uint32_t _id, uint32_t _guid);

    void loadInstanceEncounters();
    DungeonEncounterList const* getDungeonEncounterList(uint32_t _mapId, uint8_t _difficulty);

    void loadCreatureMovementOverrides();
    void checkCreatureMovement(uint32_t _id, CreatureMovementData& _creatureMovement);
    CreatureMovementData const* getCreatureMovementOverride(uint32_t _spawnId) const;

    void loadWorldStateTemplates();
    std::shared_ptr<WorldStateMap> getWorldStatesForMap(uint32_t _map) const;

    void loadCreatureTimedEmotes();
    std::shared_ptr<TimedEmoteList> getTimedEmoteList(uint32_t _spawnId);

    void generateLevelUpInfo();
    std::shared_ptr<LevelInfo> getLevelInfo(uint32_t _race, uint32_t _class, uint32_t _level);

    Pet* createPet(uint32_t _entry, WDB::Structures::SummonPropertiesEntry const* properties);
    void loadPetSpellCooldowns();
    uint32_t getPetSpellCooldown(uint32_t _spellId);

    Item* loadItem(uint32_t _lowGuid);
    Item* createItem(uint32_t _entry, Player* _playerOwner);

    void setHighestGuids();
    uint32_t generateReportId();
    uint32_t generateEquipmentSetId();
    uint32_t generateMailId();
    uint32_t generateLowGuid(uint32_t _guidHigh);
    uint32_t generateArenaTeamId();
    uint32_t generateGuildId();
    uint32_t generateCreatureSpawnId();
    uint32_t generateGameObjectSpawnId();
    uint32_t generateItemPageEntry();
#if VERSION_STRING > WotLK
    uint64_t generateVoidStorageItemId();
#endif

private:
    std::unordered_map<uint32_t, std::shared_ptr<std::vector<TrainerSpell>>> m_trainerSpellSet;
    std::unordered_map<uint32_t, std::shared_ptr<Trainer>> m_trainers;
    std::unordered_map<uint32_t, CreatureDisplayInfoData> m_creatureDisplayInfoData;
    std::unordered_map<uint32_t, DungeonEncounterList>  m_dungeonEncounterStore;
    std::unordered_map<uint32_t, CreatureMovementData> m_creatureMovementOverrides;
    std::map<uint32_t, std::shared_ptr<WorldStateMap>> m_worldstateTemplates;
    std::unordered_map<uint32_t, std::shared_ptr<TimedEmoteList>> m_timedEmotes;

    typedef std::map<uint32_t, std::shared_ptr<LevelInfo>> LevelMap;
    typedef std::map<std::pair<uint32_t, uint32_t>, std::shared_ptr<LevelMap>> LevelInfoMap;
    LevelInfoMap m_levelInfo;

    std::map<uint32_t, uint32_t> m_petSpellCooldowns;

protected:
    std::atomic<unsigned long> m_hiItemGuid = 0;
    std::atomic<unsigned long> m_hiItemPageEntry = 0;
    std::atomic<unsigned long> m_hiGroupId = 0;
    std::atomic<unsigned long> m_hiCharterId = 0;
    std::atomic<unsigned long> m_hiCreatureSpawnId;
    std::atomic<unsigned long> m_hiGameObjectSpawnId;
    std::atomic<unsigned long> m_mailId = 0;
    std::atomic<unsigned long> m_reportId = 0;
    std::atomic<unsigned long> m_setGuid = 0;
    std::atomic<unsigned long> m_hiCorpseGuid = 0;
    std::atomic<unsigned long> m_hiGuildId = 0;
    std::atomic<unsigned long> m_hiPetGuid = 0;
    std::atomic<unsigned long> m_hiArenaTeamId = 0;
    std::atomic<unsigned long> m_hiPlayerGuid = 1;
#if VERSION_STRING > WotLK
    std::atomic<unsigned long> m_voidItemId = 1;
#endif
};

#define sObjectMgr ObjectMgr::getInstance()
