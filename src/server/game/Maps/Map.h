/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINITY_MAP_H
#define TRINITY_MAP_H

#include "Define.h"

#include "Cell.h"
#include "DatabaseEnvFwd.h"
#include "DynamicTree.h"
#include "GridDefines.h"
#include "GridRefManager.h"
#include "GroupInstanceReference.h"
#include "MapDefines.h"
#include "MapDeferredBuffer.h"
#include "MapPartitioning.h"
#include "MapReference.h"
#include "MapRefManager.h"
#include "MPSCQueue.h"
#include "ObjectGuid.h"
#include "ObjectGuidSequenceGenerator.h"
#include "PartitionWorkerPool.h"
#include "PersonalPhaseTracker.h"
#include "SharedDefines.h"
#include "SpawnData.h"
#include "Timer.h"
#include <boost/container/small_vector.hpp>
#include "UniqueTrackablePtr.h"
#include "WorldStateDefines.h"
#include <bitset>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <unordered_set>

class Battleground;
class BattlegroundMap;
class BattlegroundScript;
class CreatureGroup;
class GameObjectModel;
class Group;
class InstanceLock;
class InstanceMap;
class InstanceScript;
class InstanceScenario;
class Object;
class PhaseShift;
class Player;
class SpawnedPoolData;
class TempSummon;
class TerrainInfo;
class Unit;
class Weather;
class WorldObject;
class WorldPacket;
struct DungeonEncounterEntry;
struct MapDifficultyEntry;
struct MapEntry;
struct Position;
struct ScriptAction;
struct ScriptInfo;
struct SmoothPhasingInfo;
struct SummonPropertiesEntry;
struct UpdateAdditionalSaveDataEvent;
struct UpdateBossStateSaveDataEvent;
class Transport;
enum Difficulty : uint8;
enum WeatherState : uint32;
enum class ItemContext : uint8;

namespace Trinity { struct ObjectUpdater; }
namespace Vignettes { struct VignetteData; }
namespace VMAP { enum class ModelIgnoreFlags : uint32; }

enum TransferAbortReason : uint32
{
    TRANSFER_ABORT_NONE                          = 0,
    TRANSFER_ABORT_ERROR                         = 1,
    TRANSFER_ABORT_MAX_PLAYERS                   = 2,   // Transfer Aborted: instance is full
    TRANSFER_ABORT_NOT_FOUND                     = 3,   // Transfer Aborted: instance not found
    TRANSFER_ABORT_TOO_MANY_INSTANCES            = 4,   // You have entered too many instances recently.
    TRANSFER_ABORT_ZONE_IN_COMBAT                = 6,   // Unable to zone in while an encounter is in progress.
    TRANSFER_ABORT_INSUF_EXPAN_LVL               = 7,   // You must have <TBC, WotLK> expansion installed to access this area.
    TRANSFER_ABORT_DIFFICULTY                    = 8,   // <Normal, Heroic, Epic> difficulty mode is not available for %s.
    TRANSFER_ABORT_UNIQUE_MESSAGE                = 9,   // Until you've escaped TLK's grasp, you cannot leave this place!
    TRANSFER_ABORT_TOO_MANY_REALM_INSTANCES      = 10,  // Additional instances cannot be launched, please try again later.
    TRANSFER_ABORT_NEED_GROUP                    = 11,  // Transfer Aborted: you must be in a raid group to enter this instance
    TRANSFER_ABORT_NOT_FOUND_2                   = 12,  // Transfer Aborted: instance not found
    TRANSFER_ABORT_NOT_FOUND_3                   = 13,  // Transfer Aborted: instance not found
    TRANSFER_ABORT_NOT_FOUND_4                   = 14,  // Transfer Aborted: instance not found
    TRANSFER_ABORT_REALM_ONLY                    = 15,  // All players in the party must be from the same realm to enter %s.
    TRANSFER_ABORT_MAP_NOT_ALLOWED               = 16,  // Map cannot be entered at this time.
    TRANSFER_ABORT_LOCKED_TO_DIFFERENT_INSTANCE  = 18,  // You are already locked to %s
    TRANSFER_ABORT_ALREADY_COMPLETED_ENCOUNTER   = 19,  // You are ineligible to participate in at least one encounter in this instance because you are already locked to an instance in which it has been defeated.
    TRANSFER_ABORT_DIFFICULTY_NOT_FOUND          = 22,  // client writes to console "Unable to resolve requested difficultyID %u to actual difficulty for map %d"
    TRANSFER_ABORT_XREALM_ZONE_DOWN              = 24,  // Transfer Aborted: cross-realm zone is down
    TRANSFER_ABORT_SOLO_PLAYER_SWITCH_DIFFICULTY = 26,  // This instance is already in progress. You may only switch difficulties from inside the instance.
    TRANSFER_ABORT_NOT_CROSS_FACTION_COMPATIBLE  = 33,  // This instance isn't available for cross-faction groups
};

struct TransferAbortParams
{
    TransferAbortParams(TransferAbortReason reason, uint8 arg = 0, int32 mapDifficultyXConditionId = 0)
        : Reason(reason), Arg(arg), MapDifficultyXConditionId(mapDifficultyXConditionId)
    {
    }

    TransferAbortReason Reason;
    uint8 Arg;
    int32 MapDifficultyXConditionId;

    operator bool() const { return Reason != TRANSFER_ABORT_NONE; }
};

struct ScriptAction
{
    ObjectGuid sourceGUID;
    ObjectGuid targetGUID;
    ObjectGuid ownerGUID;                                   ///> owner of source if source is item
    ScriptInfo const* script;                               ///> pointer to static script data
};

struct ZoneDynamicInfo
{
    ZoneDynamicInfo();

    uint32 MusicId;

    std::unique_ptr<Weather> DefaultWeather;
    WeatherState WeatherId;
    float Intensity;

    struct LightOverride
    {
        uint32 AreaLightId;
        uint32 OverrideLightId;
        uint32 TransitionMilliseconds;
    };
    std::vector<LightOverride> LightOverrides;
};

#define MIN_UNLOAD_DELAY      1                             // immediate unload
#define MAP_INVALID_ZONE      0xFFFFFFFF

struct RespawnInfo; // forward declaration
struct CompareRespawnInfo
{
    bool operator()(RespawnInfo const* a, RespawnInfo const* b) const;
};
using ZoneDynamicInfoMap = std::unordered_map<uint32 /*zoneId*/, ZoneDynamicInfo>;
struct RespawnListContainer;
using RespawnInfoMap = std::unordered_map<ObjectGuid::LowType, RespawnInfo*>;
struct TC_GAME_API RespawnInfo
{
    virtual ~RespawnInfo();

    SpawnObjectType type;
    ObjectGuid::LowType spawnId;
    uint32 entry;
    time_t respawnTime;
    uint32 gridId;
};
inline bool CompareRespawnInfo::operator()(RespawnInfo const* a, RespawnInfo const* b) const
{
    if (a == b)
        return false;
    if (a->respawnTime != b->respawnTime)
        return (a->respawnTime > b->respawnTime);
    if (a->spawnId != b->spawnId)
        return a->spawnId < b->spawnId;
    ASSERT(a->type != b->type, "Duplicate respawn entry for spawnId (%u," UI64FMTD ") found!", a->type, a->spawnId);
    return a->type < b->type;
}

template <typename ObjectType>
struct MapStoredObjectsUnorderedMap
{
    using Container = std::unordered_map<ObjectGuid, ObjectType*>;
    using KeyType = ObjectGuid;
    using ValueType = ObjectType*;

    static bool Insert(Container& container, ValueType object)
    {
        auto [itr, isNew] = container.try_emplace(object->GetGUID(), object);
        ASSERT(isNew || itr->second == object, "Object with certain key already in but objects are different!");
        return true;
    }

    static bool Remove(Container& container, ValueType object)
    {
        container.erase(object->GetGUID());
        return true;
    }

    static std::size_t Size(Container const& container)
    {
        return container.size();
    }

    static ValueType Find(Container const& container, KeyType const& key)
    {
        auto itr = container.find(key);
        return itr != container.end() ? itr->second : nullptr;
    }
};

extern template struct TypeListContainer<MapStoredObjectsUnorderedMap, Creature, GameObject, DynamicObject, Pet, Corpse, AreaTrigger, SceneObject, Conversation>;
using MapStoredObjectTypesContainer = TypeListContainer<MapStoredObjectsUnorderedMap, Creature, GameObject, DynamicObject, Pet, Corpse, AreaTrigger, SceneObject, Conversation>;

class TC_GAME_API Map : public GridRefManager<NGridType>
{
    friend class MapReference;
    public:
        Map(uint32 id, time_t, uint32 InstanceId, Difficulty SpawnMode, uint32 layerId = 0);
        virtual ~Map();

        MapEntry const* GetEntry() const { return i_mapEntry; }

        // currently unused for normal maps
        bool CanUnload(uint32 diff)
        {
            if (!m_unloadTimer)
                return false;

            if (m_unloadTimer <= diff)
                return true;

            m_unloadTimer -= diff;
            return false;
        }

        virtual bool AddPlayerToMap(Player* player, bool initPlayer = true);
        virtual void RemovePlayerFromMap(Player*, bool);

        template<class T> bool AddToMap(T *);
        template<class T> void RemoveFromMap(T *, bool);

        // Phase 3 (ARGUSCORE_FIXES.md, "real concurrent fan-out") - shard is which
        // _markedCellsByShard slot this call dedups/marks against: an interior shard index
        // [0, GetShardCount()) when called from that shard's own fan-out task (safe - interior
        // classification guarantees this call can never touch a cell any other shard's task is
        // concurrently touching), or the sentinel value GetShardCount() itself for the serial
        // "boundary" pass (dedups against every shard's bits, since by the time it runs every
        // interior task has already joined). On an unpartitioned map every call just passes 0.
        void VisitNearbyCellsOf(WorldObject* obj, TypeContainerVisitor<Trinity::ObjectUpdater, GridTypeMapContainer> &gridVisitor, TypeContainerVisitor<Trinity::ObjectUpdater, WorldTypeMapContainer> &worldVisitor, uint32 shard);
        virtual void Update(uint32);

        float GetVisibilityRange() const { return m_VisibleDistance; }
        //function for setting up visibility distance for maps on per-type/per-Id basis
        virtual void InitVisibilityDistance();

        void PlayerRelocation(Player*, float x, float y, float z, float orientation);
        void CreatureRelocation(Creature* creature, float x, float y, float z, float ang, bool respawnRelocationOnFail = true);
        void GameObjectRelocation(GameObject* go, float x, float y, float z, float orientation, bool respawnRelocationOnFail = true);
        void DynamicObjectRelocation(DynamicObject* go, float x, float y, float z, float orientation);
        void AreaTriggerRelocation(AreaTrigger* at, float x, float y, float z, float orientation);

        template<class T, class CONTAINER>
        void Visit(Cell const& cell, TypeContainerVisitor<T, CONTAINER>& visitor);

        bool IsRemovalGrid(float x, float y) const
        {
            GridCoord p = Trinity::ComputeGridCoord(x, y);
            return !getNGrid(p.x_coord, p.y_coord) || getNGrid(p.x_coord, p.y_coord)->GetGridState() == GRID_STATE_REMOVAL;
        }
        bool IsRemovalGrid(Position const& pos) const { return IsRemovalGrid(pos.GetPositionX(), pos.GetPositionY()); }

        bool IsGridLoaded(uint32 gridId) const { return IsGridLoaded(GridCoord(gridId % MAX_NUMBER_OF_GRIDS, gridId / MAX_NUMBER_OF_GRIDS)); }
        bool IsGridLoaded(float x, float y) const { return IsGridLoaded(Trinity::ComputeGridCoord(x, y)); }
        bool IsGridLoaded(Position const& pos) const { return IsGridLoaded(pos.GetPositionX(), pos.GetPositionY()); }

        bool GetUnloadLock(GridCoord const& p) const { return getNGrid(p.x_coord, p.y_coord)->getUnloadLock(); }
        void SetUnloadLock(GridCoord const& p, bool on) { getNGrid(p.x_coord, p.y_coord)->setUnloadExplicitLock(on); }
        void LoadGrid(float x, float y);
        void LoadGridForActiveObject(float x, float y, WorldObject const* object);
        void LoadAllCells();
        bool UnloadGrid(NGridType& ngrid, bool pForce);
        void GridMarkNoUnload(uint32 x, uint32 y);
        void GridUnmarkNoUnload(uint32 x, uint32 y);
        void UnloadAll();

        void ResetGridExpiry(NGridType &grid, float factor = 1) const
        {
            grid.ResetTimeTracker(time_t(float(i_gridExpiry)*factor));
        }

        time_t GetGridExpiry() const { return i_gridExpiry; }

        static void InitStateMachine();
        static void DeleteStateMachine();

        TerrainInfo* GetTerrain() const { return m_terrain.get(); }

        // custom PathGenerator include and exclude filter flags
        // these modify what kind of terrain types are available in current instance
        // for example this can be used to mark offmesh connections as enabled/disabled
        uint16 GetForceEnabledNavMeshFilterFlags() const { return m_forceEnabledNavMeshFilterFlags; }
        void SetForceEnabledNavMeshFilterFlag(uint16 flag) { m_forceEnabledNavMeshFilterFlags |= flag; }
        void RemoveForceEnabledNavMeshFilterFlag(uint16 flag) { m_forceEnabledNavMeshFilterFlags &= ~flag; }

        uint16 GetForceDisabledNavMeshFilterFlags() const { return m_forceDisabledNavMeshFilterFlags; }
        void SetForceDisabledNavMeshFilterFlag(uint16 flag) { m_forceDisabledNavMeshFilterFlags |= flag; }
        void RemoveForceDisabledNavMeshFilterFlag(uint16 flag) { m_forceDisabledNavMeshFilterFlags &= ~flag; }

        void GetFullTerrainStatusForPosition(PhaseShift const& phaseShift, float x, float y, float z, PositionFullTerrainStatus& data, Optional<map_liquidHeaderTypeFlags> reqLiquidType = {}, float collisionHeight = 2.03128f); // DEFAULT_COLLISION_HEIGHT in Object.h
        ZLiquidStatus GetLiquidStatus(PhaseShift const& phaseShift, float x, float y, float z, Optional<map_liquidHeaderTypeFlags> ReqLiquidType = {}, LiquidData* data = nullptr, float collisionHeight = 2.03128f); // DEFAULT_COLLISION_HEIGHT in Object.h

        uint32 GetAreaId(PhaseShift const& phaseShift, float x, float y, float z);
        uint32 GetAreaId(PhaseShift const& phaseShift, Position const& pos) { return GetAreaId(phaseShift, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ()); }
        uint32 GetZoneId(PhaseShift const& phaseShift, float x, float y, float z);
        uint32 GetZoneId(PhaseShift const& phaseShift, Position const& pos) { return GetZoneId(phaseShift, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ()); }
        void GetZoneAndAreaId(PhaseShift const& phaseShift, uint32& zoneid, uint32& areaid, float x, float y, float z);
        void GetZoneAndAreaId(PhaseShift const& phaseShift, uint32& zoneid, uint32& areaid, Position const& pos) { GetZoneAndAreaId(phaseShift, zoneid, areaid, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ()); }

        float GetMinHeight(PhaseShift const& phaseShift, float x, float y);
        float GetGridHeight(PhaseShift const& phaseShift, float x, float y);
        float GetStaticHeight(PhaseShift const& phaseShift, float x, float y, float z, bool checkVMap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH);
        float GetStaticHeight(PhaseShift const& phaseShift, Position const& pos, bool checkVMap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) { return GetStaticHeight(phaseShift, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), checkVMap, maxSearchDist); }
        float GetHeight(PhaseShift const& phaseShift, float x, float y, float z, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) { return std::max<float>(GetStaticHeight(phaseShift, x, y, z, vmap, maxSearchDist), GetGameObjectFloor(phaseShift, x, y, z, maxSearchDist)); }
        float GetHeight(PhaseShift const& phaseShift, Position const& pos, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) { return GetHeight(phaseShift, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), vmap, maxSearchDist); }

        float GetWaterLevel(PhaseShift const& phaseShift, float x, float y);
        bool IsInWater(PhaseShift const& phaseShift, float x, float y, float z, LiquidData* data = nullptr);
        bool IsUnderWater(PhaseShift const& phaseShift, float x, float y, float z);

        float GetWaterOrGroundLevel(PhaseShift const& phaseShift, float x, float y, float z, float* ground = nullptr, bool swim = false, float collisionHeight = 2.03128f); // DEFAULT_COLLISION_HEIGHT in Object.h

        void MoveAllCreaturesInMoveList();
        void MoveAllGameObjectsInMoveList();
        void MoveAllDynamicObjectsInMoveList();
        void MoveAllAreaTriggersInMoveList();
        void RemoveAllObjectsInRemoveList();
        virtual void RemoveAllPlayers();

        // used only in MoveAllCreaturesInMoveList and ObjectGridUnloader
        bool CreatureRespawnRelocation(Creature* c, bool diffGridOnly);
        bool GameObjectRespawnRelocation(GameObject* go, bool diffGridOnly);

        // assert print helper
        template <typename T>
        static bool CheckGridIntegrity(T* object, bool moved, char const* objType);

        uint32 GetInstanceId() const { return i_InstanceId; }
        uint32 GetWorldLayer()  const { return m_worldLayer; }

        Trinity::unique_weak_ptr<Map> GetWeakPtr() const { return m_weakRef; }
        void SetWeakPtr(Trinity::unique_weak_ptr<Map> weakRef) { m_weakRef = std::move(weakRef); }

        static TransferAbortParams PlayerCannotEnter(uint32 mapid, Player* player);
        virtual TransferAbortParams CannotEnter(Player* /*player*/) { return { TRANSFER_ABORT_NONE }; }
        char const* GetMapName() const;

        // have meaning only for instanced map (that have set real difficulty)
        Difficulty GetDifficultyID() const { return Difficulty(i_spawnMode); }
        MapDifficultyEntry const* GetMapDifficulty() const;

        uint32 GetId() const;
        bool Instanceable() const;
        bool IsDungeon() const;
        bool IsNonRaidDungeon() const;
        bool IsRaid() const;
        bool IsLFR() const;
        bool IsNormal() const;
        bool IsHeroic() const;
        bool IsMythic() const;
        bool IsMythicPlus() const;
        bool IsHeroicOrHigher() const;
        bool Is25ManRaid() const;
        bool IsTimewalking() const;
        bool IsBattleground() const;
        bool IsBattleArena() const;
        bool IsBattlegroundOrArena() const;
        bool IsScenario() const;
        bool IsGarrison() const;
        // Currently, this means that every entity added to this map will be marked as active
        bool IsAlwaysActive() const;

        // Returns a short stable string for use in metric tags — never allocates.
        char const* GetMapTypeName() const
        {
            if (IsRaid())               return "raid";
            if (IsBattlegroundOrArena()) return "bg";
            if (IsScenario())           return "scenario";
            if (IsDungeon())            return "dungeon";
            return "world";
        }
        bool GetEntrancePos(int32& mapid, float& x, float& y);

        void AddObjectToRemoveList(WorldObject* obj);
        void AddObjectToSwitchList(WorldObject* obj, bool on);
        virtual void DelayedUpdate(uint32 diff);

        // Phase 3 (ARGUSCORE_FIXES.md) - _markedCellsByShard replaces the old single flat bitset.
        // isCellMarkedInShard/markCellInShard are what VisitNearbyCellsOf itself uses per-call
        // (see its own declaration above for what "shard" means there); isCellMarkedAnyShard is
        // what ProcessRelocationNotifies reads - correct (not merely harmlessly redundant) given
        // interior shards are disjoint by construction and the boundary pass already dedups
        // against every interior shard before writing its own trailing slot.
        void resetMarkedCells() { for (auto& bs : _markedCellsByShard) bs.reset(); }
        bool isCellMarkedInShard(uint32 shard, uint32 pCellId) const { return _markedCellsByShard[shard].test(pCellId); }
        void markCellInShard(uint32 shard, uint32 pCellId) { _markedCellsByShard[shard].set(pCellId); }
        bool isCellMarkedAnyShard(uint32 pCellId) const
        {
            for (auto const& bs : _markedCellsByShard)
                if (bs.test(pCellId))
                    return true;
            return false;
        }

        bool HavePlayers() const { return !m_mapRefManager.empty(); }
        uint32 GetPlayersCountExceptGMs() const;
        bool ActiveObjectsNearGrid(NGridType const& ngrid) const;

        void AddWorldObject(WorldObject* obj);
        void RemoveWorldObject(WorldObject* obj);

        void SendToPlayers(WorldPacket const* data) const;

        using PlayerList = MapRefManager;
        PlayerList const& GetPlayers() const { return m_mapRefManager; }

        template <typename T>
        void DoOnPlayers(T&& fn)
        {
            for (MapReference const& ref : GetPlayers())
                if (Player* player = ref.GetSource())
                    fn(player);
        }

        //per-map script storage
        void ScriptsStart(std::map<uint32, std::multimap<uint32, ScriptInfo>> const& scripts, uint32 id, Object* source, Object* target);
        void ScriptCommandStart(ScriptInfo const& script, uint32 delay, Object* source, Object* target);

        // must called with AddToWorld
        void AddToActive(WorldObject* obj);

        // must called with RemoveFromWorld
        void RemoveFromActive(WorldObject* obj);

        template<class T> void SwitchGridContainers(T* obj, bool on);

        // Phase 3 redesign, Stage 4 (ARGUSCORE_FIXES.md) - CreatureGroupHolder used to be a
        // public raw container FormationMgr::AddCreatureToGroup/RemoveCreatureFromGroup
        // try_emplace'd/erase'd directly, reachable from a fan-out worker thread via ordinary
        // creature spawn/despawn with no synchronization - real unordered_map corruption. No
        // longer exposed directly (same "encapsulate, don't leak the raw container" shape as
        // A11's AreaTrigger spawn-id store).
        //
        // Stage 4 fix (ARGUSCORE_FIXES.md, review finding) - originally two separate locked
        // helpers (find-or-create, then a separate erase), with FormationMgr calling CreatureGroup
        // member-mutation methods (their own separately-locked recursive_mutex) in between. That
        // let the holder lookup/erase and the group's own member add/remove interleave across two
        // different threads: thread A empties and deletes a formation's last-member group between
        // thread B's holder lookup (finds the not-yet-erased group) and thread B's AddMember call
        // on it - a real use-after-free. These two methods now do the WHOLE find-or-create+add and
        // remove+maybe-erase+delete sequence each, atomically, under one lock held for the entire
        // call - FormationMgr no longer touches the holder or orchestrates the two lock scopes
        // itself.
        CreatureGroup* AddCreatureToFormation(ObjectGuid::LowType leaderSpawnId, Creature* creature);
        void RemoveCreatureFromFormation(CreatureGroup* group, Creature* member);

        void UpdateIteratorBack(Player* player);

        TempSummon* SummonCreature(uint32 entry, Position const& pos, SummonPropertiesEntry const* properties = nullptr, Milliseconds duration = 0ms, WorldObject* summoner = nullptr, uint32 spellId = 0, uint32 vehId = 0, ObjectGuid privateObjectOwner = ObjectGuid::Empty, SmoothPhasingInfo const* smoothPhasingInfo = nullptr);
        void SummonCreatureGroup(uint8 group, std::list<TempSummon*>* list = nullptr);
        AreaTrigger* GetAreaTrigger(ObjectGuid const& guid);
        SceneObject* GetSceneObject(ObjectGuid const& guid);
        Conversation* GetConversation(ObjectGuid const& guid);
        Player* GetPlayer(ObjectGuid const& guid);
        Corpse* GetCorpse(ObjectGuid const& guid);
        Creature* GetCreature(ObjectGuid const& guid);
        DynamicObject* GetDynamicObject(ObjectGuid const& guid);
        GameObject* GetGameObject(ObjectGuid const& guid);
        Pet* GetPet(ObjectGuid const& guid);
        Transport* GetTransport(ObjectGuid const& guid);
        Creature* GetCreatureBySpawnId(ObjectGuid::LowType spawnId) const;
        GameObject* GetGameObjectBySpawnId(ObjectGuid::LowType spawnId) const;
        AreaTrigger* GetAreaTriggerBySpawnId(ObjectGuid::LowType spawnId) const;
        WorldObject* GetWorldObjectBySpawnId(SpawnObjectType type, ObjectGuid::LowType spawnId) const
        {
            switch (type)
            {
                case SPAWN_TYPE_CREATURE:
                    return reinterpret_cast<WorldObject*>(GetCreatureBySpawnId(spawnId));
                case SPAWN_TYPE_GAMEOBJECT:
                    return reinterpret_cast<WorldObject*>(GetGameObjectBySpawnId(spawnId));
                case SPAWN_TYPE_AREATRIGGER:
                    return reinterpret_cast<WorldObject*>(GetAreaTriggerBySpawnId(spawnId));
                default:
                    return nullptr;
            }
        }

        // Sharded by live position at insert time (Phase 2 of the "Continent Map Spatial
        // Partitioning" design - see ARGUSCORE_FIXES.md). One shard (index 0) for an
        // unpartitioned map - the common case today, byte-for-byte equivalent to the single
        // flat container this replaces. Unlike Phase 1's by-spawnId stores, most of the 8 types
        // held here (DynamicObject/Pet/Corpse/SceneObject/Conversation, plus non-static
        // AreaTriggers) have no static home position and several move after creation, so the
        // shard a GUID was actually inserted into is tracked in _objectShardIndex rather than
        // recomputed from current position on every remove/lookup - guarantees insert/remove
        // always agree regardless of how far an object has since moved. Raw container access
        // (this used to return MapStoredObjectTypesContainer& directly) is intentionally not
        // exposed anymore - every caller goes through these instead.
        template <typename ObjectType>
        void AddToObjectsStore(ObjectType* obj)
        {
            uint32 shard = GetPartitionIndexForPosition(obj->GetPositionX(), obj->GetPositionY());
            {
                std::unique_lock<std::shared_mutex> lock(_objectsStoreLock);
                _objectsStore[shard].Insert<ObjectType>(obj);
            }
            std::unique_lock<std::shared_mutex> lock(_objectShardIndexLock);
            _objectShardIndex[obj->GetGUID()] = shard;
        }

        template <typename ObjectType>
        void RemoveFromObjectsStore(ObjectType* obj)
        {
            ObjectGuid guid = obj->GetGUID();
            uint32 shard;
            {
                std::unique_lock<std::shared_mutex> lock(_objectShardIndexLock);
                auto it = _objectShardIndex.find(guid);
                shard = it != _objectShardIndex.end() ? it->second : 0;
                if (it != _objectShardIndex.end())
                    _objectShardIndex.erase(it);
            }
            std::unique_lock<std::shared_mutex> lock(_objectsStoreLock);
            _objectsStore[shard].Remove<ObjectType>(obj);
        }

        template <typename ObjectType>
        ObjectType* GetWorldObjectInObjectsStore(ObjectGuid const& guid) const
        {
            uint32 shard;
            {
                std::shared_lock<std::shared_mutex> lock(_objectShardIndexLock);
                auto it = _objectShardIndex.find(guid);
                if (it == _objectShardIndex.end())
                    return nullptr;
                shard = it->second;
            }
            std::shared_lock<std::shared_mutex> lock(_objectsStoreLock);
            return _objectsStore[shard].Find<ObjectType>(guid);
        }

        template <typename ObjectType>
        std::size_t GetObjectsStoreSize() const
        {
            std::size_t total = 0;
            for (MapStoredObjectTypesContainer const& shard : _objectsStore)
                total += shard.Size<ObjectType>();
            return total;
        }

        // Narrow escape hatch for the handful of callers that need to walk every stored object
        // of every type at once (TypeContainerVisitor-style generic visitors) - deliberately
        // still exposes the per-shard container by reference rather than pretending there's
        // only one, but through indexed shard access rather than a single flat reference, so a
        // caller can no longer accidentally treat this map as unpartitioned. `.Data` itself
        // remains part of TypeListContainer's own sanctioned, type-checked public interface
        // (`.Data.FindContainer<T>()`) - what's not supported anymore is reaching into
        // `.Data.Head` directly and assuming a specific type occupies that slot.
        uint32 GetObjectsStoreShardCount() const { return uint32(_objectsStore.size()); }
        MapStoredObjectTypesContainer& GetObjectsStoreShard(uint32 shard) { return _objectsStore[shard]; }
        MapStoredObjectTypesContainer const& GetObjectsStoreShard(uint32 shard) const { return _objectsStore[shard]; }

        // Which partition currently owns a live WorldObject's position - the public entry point
        // for external, position-based partition lookups (Phase 3 - see ARGUSCORE_FIXES.md;
        // PathGenerator uses this for per-partition dtNavMeshQuery isolation). Deliberately a
        // narrow, WorldObject-scoped wrapper rather than exposing the raw position-based
        // GetPartitionIndexForPosition(float,float) publicly, to keep partition-math callers
        // going through a real live object rather than ad-hoc coordinates. Defined in Map.cpp,
        // not inline here, despite being a one-line body - it dereferences WorldObject const*
        // (GetPositionX/GetPositionY), and Map.h deliberately only forward-declares WorldObject
        // (never includes Object.h/WorldObject.h). Unlike the template methods elsewhere in this
        // class, this is an ordinary non-template member function, so its body would be parsed -
        // requiring a complete WorldObject - at the point Map.h itself is included, not deferred
        // to instantiation. CellImpl.h includes Map.h before Object.h, and Map.cpp/MapScripts.cpp
        // both include Map.h as their literal first line, so this is a real, immediately-hit
        // compile failure if defined inline, not a theoretical one.
        uint32 GetPartitionIndexForObject(WorldObject const* obj) const;

        // Which shard currently "owns" a live object's bookkeeping - _objectShardIndex (Phase 2)
        // for the 8 types tracked in _objectsStore, a live position recompute (the only option
        // available) for anything not tracked there, most notably Player. This is deliberately
        // NOT the same thing as "where is this object standing right now" for tracked types -
        // nothing updates _objectShardIndex after insertion until Phase 4 (cross-partition
        // object transfer) exists, so the cached shard is the correct source of truth for "which
        // partition-thread's containers this object's entry actually lives in", even if that
        // temporarily disagrees with current position. See ARGUSCORE_FIXES.md, cross-partition
        // combat/damage/aura safety design. Defined in Map.cpp for the same incomplete-WorldObject
        // reason as GetPartitionIndexForObject above.
        uint32 ShardOf(WorldObject const* obj) const;

        // Do two live objects currently belong to different partitions? False immediately on an
        // unpartitioned map (GetShardCount()==1), without even a ShardOf lookup - the
        // common-case fast path every guard built on this must preserve. This is the entry
        // point ThreatManager::AddThreat/CombatManager::SetInCombatWith/Unit::DealDamage/
        // Aura::UpdateTargetMap check before proceeding synchronously vs. deferring - see
        // ARGUSCORE_FIXES.md. Defined in Map.cpp for the same incomplete-WorldObject reason as
        // GetPartitionIndexForObject above.
        bool IsCrossPartition(WorldObject const* a, WorldObject const* b) const;

        // Code-review deep-dive fix (ARGUSCORE_FIXES.md) - a second, complementary primitive to
        // IsCrossPartition(a, b) for the specific class of guard IsCrossPartition can't cleanly
        // express: a function that needs to know "is it safe for the thread I'm ACTUALLY
        // executing on right now to touch obj", but has no clean single "real anchor" WorldObject
        // parameter of its own to compare obj against (e.g. ThreatManager::ForwardThreatForAssistingMe's
        // `assistant`, or Unit::ProcSkillsAndAuras's `actor`, both of which can be a THIRD PARTY -
        // e.g. Spell::m_originalCaster - never validated against whichever object's thread is
        // really running this code). Rather than pushing that anchor-tracking burden onto every
        // caller (which is exactly how those two functions' own guards ended up duplicated at
        // multiple call sites, each needing to independently know its own real anchor), this asks
        // the one thing that's ALWAYS available without any anchor at all:
        // CurrentFanOutShardForThisMap() (which PartitionWorkerPool task is actually executing on
        // THIS OS thread right now, set once for the whole task regardless of call depth or which
        // object's method nominally "owns" the current call) compared directly against obj's own
        // recorded _tickDispatchShard entry (see its own comment) - no second object needed.
        // Returns false (safe) whenever nothing is currently fanning out on this thread at all
        // (single-threaded right now, trivially safe regardless of obj). Falls back to
        // ShardOf(obj) when obj wasn't classified this tick (rare - e.g. spawned after
        // classification already ran), matching IsCrossPartition's own fallback style.
        bool IsUnsafeForCurrentThreadToTouch(WorldObject const* obj) const;

        // Phase 6 (ARGUSCORE_FIXES.md, "Double-buffered halo snapshots") - is the given live
        // position within haloWidth of a partition other than its own? False immediately on an
        // unpartitioned map - GetShardCount()<=1, the actual fast-path convention
        // IsCrossPartition/GetPartitionIndexForGrid use, deliberately NOT "!_cachedPartitionLayout"
        // (an independent review caught these as subtly different: a legal-but-degenerate 1x1
        // Map.Partitioning.MapIds entry populates _cachedPartitionLayout with a single-partition
        // layout, so GetPartitionCount()==1 and GetShardCount()==1 - "unpartitioned" by every
        // other check in this class - while _cachedPartitionLayout itself is still set; checking
        // presence rather than count would incorrectly skip the fast path for that config). Public
        // (not private like GetPartitionIndexForGrid) - called from HaloSnapshotPublishWorker
        // (Map.cpp, anonymous namespace), a free struct rather than a Map member/deferred-lambda-
        // inside-a-Map-member, so it needs real public access rather than relying on
        // class-internal visibility. Deliberately takes a live x/y rather than an object - the
        // publish worker already has the object's current position on hand from its own
        // iteration, avoiding a redundant virtual call.
        bool IsNearPartitionBoundary(float x, float y, float haloWidth) const
        {
            // GetShardCount()<=1 short-circuits before the Optional check even runs, on the
            // common unpartitioned-map path (matches every sibling fast path). !_cachedPartitionLayout
            // is a second, independent guard on the actual dereference below - GetShardCount()>1
            // can only happen if the constructor's layout branch ran (which always populates
            // _cachedPartitionLayout in the same block), so this is provably unreachable today,
            // but every other _cachedPartitionLayout consumer in this class (GetPartitionIndexForPosition/
            // ForGrid) independently re-checks rather than trusting that invariant to hold forever
            // - matching that same defensive style here rather than being the one exception.
            // Phase 7 kill-switch (ARGUSCORE_FIXES.md) - additive third condition, see
            // Map::IsCrossPartition's own comment for why GetShardCount() itself stays untouched.
            if (GetShardCount() <= 1 || !_cachedPartitionLayout || sMapPartitionMgr->IsForceDisabled(GetId()))
                return false;
            return _cachedPartitionLayout->IsNearBoundary(x, y, haloWidth);
        }

        // Phase 3 (ARGUSCORE_FIXES.md, "real concurrent fan-out") - classifies a single
        // VisitNearbyCellsOf candidate (player, viewpoint, combat/aura/summon catch-up target, or
        // active non-player) as either safely confined to one shard's grid range this tick
        // ("interior", appended to interiorBuckets[shard]) or reaching across a boundary
        // ("boundary", appended to boundaryBucket - must be processed serially, never from a
        // parallel fan-out task). Uses the frozen _cachedClassificationProbeWidth (see its own
        // comment) as the IsNearPartitionBoundary probe width, NOT the object's own
        // GetGridActivationRange() (this comment previously claimed the opposite - stale/wrong,
        // corrected in a Stage 7 recheck; see ClassifyForFanOut's own comment, Map.cpp, for the
        // precise, current reasoning) and not the static Map.Partitioning.HaloWidth config value
        // either (that config is Phase 6's halo-snapshot concern, a different question) - this is
        // what makes "interior" a provable guarantee rather than a heuristic: an object classified
        // interior to shard S cannot have a CellArea reaching outside shard S's grid range this
        // tick, so two different shards' interior buckets can never contain objects whose cell
        // areas overlap - PROVIDED VisitNearbyCellsOf's own actual CellArea radius is likewise
        // clamped to this same probe width, not the object's raw GetGridActivationRange() (a
        // Stage 7 recheck finding: it wasn't, until fixed - see VisitNearbyCellsOf's own comment).
        // Silently skips null/not-in-world objects (a catch-up target can log out or despawn
        // between being gathered and being classified). Defined in Map.cpp - needs a complete
        // WorldObject, which Map.h only forward-declares.
        void ClassifyForFanOut(WorldObject* obj, std::vector<std::vector<WorldObject*>>& interiorBuckets, std::vector<WorldObject*>& boundaryBucket);

        // Moves a single tracked-type object's Phase 2 bookkeeping (_objectsStore entry +
        // _objectShardIndex) to targetShard, without touching its actual position - this is a
        // pure ownership transfer, used to "pin" a combat/threat/damage/aura pair into the same
        // partition once a cross-partition interaction is about to happen. No-op if already
        // there. ObjectType must be one of the 8 types _objectsStore actually tracks.
        template <typename ObjectType>
        void TransferObjectToPartition(ObjectType* obj, uint32 targetShard)
        {
            // Stage 9 follow-up fix (ARGUSCORE_FIXES.md, independent review finding) - this
            // mutates _objectsStore[currentShard]/_objectsStore[targetShard] with no lock of its
            // own; correctness depends entirely on every caller reaching here only through
            // ResolveCrossPartitionPair/HandleCrossPartitionTransfer, which are only ever invoked
            // from an AddFarSpellCallback replay (i.e. the serial barrier phase, no worker thread
            // active for this Map) - true today across every real call site (CombatManager.cpp,
            // ThreatManager.cpp, SpellAuraEffects.cpp, Unit.cpp, Vehicle.cpp, CreatureGroups.cpp,
            // KillRewarder.cpp, and script callers), but nothing previously caught a future call
            // site that forgot to defer first. Debug tripwire only (matches
            // GameEventMgr::StartEvent/StopEvent's own ASSERT, Stage 6) - not new locking, since
            // this genuinely should never run from a live fan-out worker thread.
            ASSERT(!IsAnyFanOutInProgressOnThisThread(), "TransferObjectToPartition called from a live fan-out worker thread - every caller must defer via AddFarSpellCallback first.");

            ObjectGuid guid = obj->GetGUID();
            uint32 currentShard = ShardOf(obj);
            if (currentShard == targetShard)
                return;

            // Same reasoning as _objectShardIndexLock just below - only ever called from the
            // barrier phase today (confirmed by the ASSERT above), so not strictly load-bearing
            // against another THIS-function caller, but a live fan-out worker thread doing an
            // ordinary GetWorldObjectInObjectsStore lookup (GetCreature/GetUnit/etc., ubiquitous
            // throughout AI/spell code) against _objectsStore[currentShard]/[targetShard] absolutely
            // can race this Remove/Insert pair without _objectsStoreLock (see its own comment) -
            // that read-side guard only works if every write-side caller, including this one,
            // consistently takes it too.
            {
                std::unique_lock<std::shared_mutex> lock(_objectsStoreLock);
                _objectsStore[currentShard].Remove<ObjectType>(obj);
                _objectsStore[targetShard].Insert<ObjectType>(obj);
            }
            // Phase 3 redesign (ARGUSCORE_FIXES.md) - see _objectShardIndexLock's own comment
            // (above _objectShardIndex's declaration). Only ever called from the barrier phase
            // today (no worker thread is active for this Map at that point), so this isn't
            // strictly load-bearing yet, but costs nothing and keeps every _objectShardIndex
            // write going through the same lock rather than leaving one silent exception a future
            // caller could trip over.
            std::unique_lock<std::shared_mutex> lock(_objectShardIndexLock);
            _objectShardIndex[guid] = targetShard;
        }

        // Generic entry point for the pin-transfer callbacks below - dispatches to the correctly
        // templated TransferObjectToPartition<T> based on obj's real runtime type, and pulls
        // every one of obj's controlled pets/guardians along with it (atomic group pinning,
        // symmetric to the vehicle+passenger group transfer already planned for Phase 4 - see
        // ARGUSCORE_FIXES.md). Player itself has no persistent bookkeeping to move (not tracked
        // in _objectsStore at all - see ShardOf's own comment) so this is a deliberate no-op for
        // the Player object itself, while still walking its controlled pets.
        void TransferWorldObjectToPartition(WorldObject* obj, uint32 targetShard);

        // Shared resolution logic for every cross-partition pin site (ThreatManager::AddThreat,
        // CombatManager::SetInCombatWith, Unit::DealDamage, Aura::UpdateTargetMap - see
        // ARGUSCORE_FIXES.md). Picks which of the two sides actually moves - prefers moving b
        // onto a's shard, falling back to moving a onto b's shard if b isn't a transferable type
        // (Player has no _objectsStore bookkeeping to move, see ShardOf's own comment). If both
        // sides are Players neither is transferable and the pair is left cross-partition - a
        // known, accepted gap (pinning two live players together means moving one of their
        // actual positions, not just bookkeeping, which is out of scope here).
        //
        // Deliberately decides the mover BEFORE calling TransferWorldObjectToPartition, rather
        // than transferring b speculatively and checking after: TransferWorldObjectToPartition
        // unconditionally drags the transferred object's controlled pets along with it (Piece 4),
        // so a speculative no-op call on a Player would still drag that player's pet to a shard
        // that then turns out not to be the final one once the a-onto-b fallback runs - a real
        // bug caught here before it shipped. Defined in Map.cpp, not inline here, for the same
        // reason as TransferWorldObjectToPartition - it needs full Unit/Pet/Creature/GameObject
        // definitions, which Map.h deliberately only forward-declares.
        void ResolveCrossPartitionPair(WorldObject* a, WorldObject* b);

        using CreatureBySpawnIdContainer = std::unordered_multimap<ObjectGuid::LowType, Creature*>;
        using GameObjectBySpawnIdContainer = std::unordered_multimap<ObjectGuid::LowType, GameObject*>;

        // Review finding (code-review deep-dive fix, ARGUSCORE_FIXES.md) - GetCreaturesBySpawnId/
        // GetGameObjectsBySpawnId used to return std::vector<T*>, a heap allocation on every call
        // even though the overwhelming common case is 0-1 matches for a given spawnId (only
        // formations/dynamic respawn scenarios legitimately produce more than a couple). Real
        // avoidable heap churn on paths called per-object during mass respawn/formation-search
        // bursts. boost::container::small_vector<T*, 4> keeps the exact same range-for/.empty()/
        // iterator interface every existing caller already uses (drop-in, zero call-site changes
        // needed beyond the handful that explicitly spelled out std::vector<T*> as a variable's
        // declared type) while storing up to 4 matches inline with no allocation at all, only
        // falling back to the heap for the genuinely rare case for further matches.
        using CreatureBySpawnIdResult = boost::container::small_vector<Creature*, 4>;
        using GameObjectBySpawnIdResult = boost::container::small_vector<GameObject*, 4>;

        // Sharded by static spawn-point partition assignment (Phase 1 of the "Continent Map
        // Spatial Partitioning" design - see ARGUSCORE_FIXES.md). One shard (index 0) for an
        // unpartitioned map - the common case today, byte-for-byte equivalent to the single
        // flat container this replaces. Raw container access is intentionally not exposed
        // anymore (this used to return CreatureBySpawnIdContainer&/GameObjectBySpawnIdContainer&
        // directly) - every caller goes through these instead, so the sharding is invisible to
        // them and no caller needs to know which shard anything lives in.
        void AddCreatureToSpawnIdStore(ObjectGuid::LowType spawnId, Creature* creature);
        void RemoveCreatureFromSpawnIdStore(ObjectGuid::LowType spawnId, Creature* creature);
        CreatureBySpawnIdResult GetCreaturesBySpawnId(ObjectGuid::LowType spawnId) const;

        void AddGameObjectToSpawnIdStore(ObjectGuid::LowType spawnId, GameObject* go);
        void RemoveGameObjectFromSpawnIdStore(ObjectGuid::LowType spawnId, GameObject* go);
        GameObjectBySpawnIdResult GetGameObjectsBySpawnId(ObjectGuid::LowType spawnId) const;

        // Phase 3 redesign, Stage 4 (ARGUSCORE_FIXES.md) - was a single unsharded container with
        // a public raw-reference getter; AreaTrigger::AddToWorld/RemoveFromWorld called insert/
        // MultimapErasePair on it directly, reachable from a fan-out worker thread with zero
        // synchronization - real unordered_multimap corruption, the same class Stage 2/3 already
        // fixed elsewhere. Sharded to match _creatureBySpawnIdStore/_gameobjectBySpawnIdStore
        // (see AddCreatureToSpawnIdStore's own comment above for why raw access is no longer
        // exposed) rather than given a mutex - same disjoint-shard-slot shape as everything else
        // this pattern already covers.
        using AreaTriggerBySpawnIdContainer = std::unordered_multimap<ObjectGuid::LowType, AreaTrigger*>;
        void AddAreaTriggerToSpawnIdStore(ObjectGuid::LowType spawnId, AreaTrigger* areaTrigger);
        void RemoveAreaTriggerFromSpawnIdStore(ObjectGuid::LowType spawnId, AreaTrigger* areaTrigger);

        std::unordered_set<Corpse*> const* GetCorpsesInCell(uint32 cellId) const
        {
            auto itr = _corpsesByCell.find(cellId);
            if (itr != _corpsesByCell.end())
                return &itr->second;

            return nullptr;
        }

        Corpse* GetCorpseByPlayer(ObjectGuid const& ownerGuid) const
        {
            auto itr = _corpsesByPlayer.find(ownerGuid);
            if (itr != _corpsesByPlayer.end())
                return itr->second;

            return nullptr;
        }

        InstanceMap* ToInstanceMap() { if (IsDungeon()) return reinterpret_cast<InstanceMap*>(this); else return nullptr;  }
        InstanceMap const* ToInstanceMap() const { if (IsDungeon()) return reinterpret_cast<InstanceMap const*>(this); return nullptr; }

        BattlegroundMap* ToBattlegroundMap() { if (IsBattlegroundOrArena()) return reinterpret_cast<BattlegroundMap*>(this); else return nullptr;  }
        BattlegroundMap const* ToBattlegroundMap() const { if (IsBattlegroundOrArena()) return reinterpret_cast<BattlegroundMap const*>(this); return nullptr; }

        bool isInLineOfSight(PhaseShift const& phaseShift, float x1, float y1, float z1, float x2, float y2, float z2, LineOfSightChecks checks, VMAP::ModelIgnoreFlags ignoreFlags) const;
        // Phase 3 redesign, Stage 4 (ARGUSCORE_FIXES.md) - _dynamicTree's own read/write split;
        // see its declaration comment below for why a shared_mutex. Balance/Remove/Insert mutate
        // the tree (unique_lock); Contains/GetGameObjectFloor and isInLineOfSight/getObjectHitPos
        // (Map.cpp) only read it (shared_lock).
        void Balance() { std::unique_lock<std::shared_mutex> lock(_dynamicTreeLock); _dynamicTree.balance(); }
        void RemoveGameObjectModel(GameObjectModel const& model) { std::unique_lock<std::shared_mutex> lock(_dynamicTreeLock); _dynamicTree.remove(model); }
        void InsertGameObjectModel(GameObjectModel const& model) { std::unique_lock<std::shared_mutex> lock(_dynamicTreeLock); _dynamicTree.insert(model); }
        bool ContainsGameObjectModel(GameObjectModel const& model) const { std::shared_lock<std::shared_mutex> lock(_dynamicTreeLock); return _dynamicTree.contains(model); }
        float GetGameObjectFloor(PhaseShift const& phaseShift, float x, float y, float z, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) const
        {
            std::shared_lock<std::shared_mutex> lock(_dynamicTreeLock);
            return _dynamicTree.getHeight(x, y, z, maxSearchDist, phaseShift);
        }
        bool getObjectHitPos(PhaseShift const& phaseShift, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float &ry, float& rz, float modifyDist);

        virtual ObjectGuid::LowType GetOwnerGuildId(uint32 /*team*/ = TEAM_OTHER) const { return UI64LIT(0); }
        /*
            RESPAWN TIMES
        */
        time_t GetLinkedRespawnTime(ObjectGuid guid) const;
        time_t GetRespawnTime(SpawnObjectType type, ObjectGuid::LowType spawnId) const
        {
            if (auto map = GetRespawnMapForType(type, GetPartitionIndexForSpawnId(type, spawnId)))
            {
                auto it = map->find(spawnId);
                return (it == map->end()) ? 0 : it->second->respawnTime;
            }
            return 0;
        }
        time_t GetCreatureRespawnTime(ObjectGuid::LowType spawnId) const { return GetRespawnTime(SPAWN_TYPE_CREATURE, spawnId); }
        time_t GetGORespawnTime(ObjectGuid::LowType spawnId) const { return GetRespawnTime(SPAWN_TYPE_GAMEOBJECT, spawnId); }

        void UpdatePlayerZoneStats(uint32 oldZone, uint32 newZone);

        void SaveRespawnTime(SpawnObjectType type, ObjectGuid::LowType spawnId, uint32 entry, time_t respawnTime, uint32 gridId, CharacterDatabaseTransaction dbTrans = nullptr, bool startup = false);
        void SaveRespawnInfoDB(RespawnInfo const& info, CharacterDatabaseTransaction dbTrans = nullptr);
        void LoadRespawnTimes();
        void DeleteRespawnTimes() { UnloadAllRespawnInfos(); DeleteRespawnTimesInDB(); }
        void DeleteRespawnTimesInDB();

        void LoadCorpseData();
        void DeleteCorpseData();
        void AddCorpse(Corpse* corpse);
        void RemoveCorpse(Corpse* corpse);
        Corpse* ConvertCorpseToBones(ObjectGuid const& ownerGuid, bool insignia = false);
        void RemoveOldCorpses();

        void SendInitTransports(Player* player);
        void SendRemoveTransports(Player* player);
        void SendUpdateTransportVisibility(Player* player);
        void SendZoneDynamicInfo(uint32 zoneId, Player* player) const;
        void SendZoneWeather(uint32 zoneId, Player* player) const;
        void SendZoneWeather(ZoneDynamicInfo const& zoneDynamicInfo, Player* player) const;

        void SetZoneMusic(uint32 zoneId, uint32 musicId);
        Weather* GetOrGenerateZoneDefaultWeather(uint32 zoneId);
        WeatherState GetZoneWeather(uint32 zoneId) const;
        void SetZoneWeather(uint32 zoneId, WeatherState weatherId, float intensity);
        void SetZoneOverrideLight(uint32 zoneId, uint32 areaLightId, uint32 overrideLightId, Milliseconds transitionTime);

        void UpdateAreaDependentAuras();

        template<HighGuid high>
        inline ObjectGuid::LowType GenerateLowGuid()
        {
            static_assert(ObjectGuidTraits<high>::SequenceSource.HasFlag(ObjectGuidSequenceSource::Map), "Only map specific guid can be generated in Map context");
            return GetGuidSequenceGenerator(high).Generate();
        }

        template<HighGuid high>
        inline ObjectGuid::LowType GetMaxLowGuid()
        {
            static_assert(ObjectGuidTraits<high>::SequenceSource.HasFlag(ObjectGuidSequenceSource::Map), "Only map specific guid can be retrieved in Map context");
            return GetGuidSequenceGenerator(high).GetNextAfterMaxUsed();
        }

        // Phase 3 redesign, Stage 2 (ARGUSCORE_FIXES.md) - buffered per-shard when called from a
        // fan-out worker thread (hit by essentially every Object::Set*Value/SetFlag call, i.e.
        // ordinary health/power/flag changes on every combat tick), merged serially right after
        // Map::Update()'s WaitAll() returns, before SendObjectUpdates() consumes _updateObjects -
        // see PerShardDeferredBuffer's own comment (MapDeferredBuffer.h). Add and remove share ONE
        // buffer (entry.second: true = add, false = remove), not two separate ones - the same
        // object can legitimately be added then removed (or vice versa) within one tick, and
        // draining "all adds, then all removes" instead of the real chronological order could
        // apply them backwards.
        //
        // PerShardDeferredBuffer::DrainInto replays shard-major (all of shard 0's entries, then
        // shard 1's, ...), NOT in global chronological order. That's only sound here because
        // every AddUpdateObject/RemoveUpdateObject call for a GIVEN object originates from that
        // object's own Update()-reachable call chain, which interior classification dispatches
        // to exactly one shard for the whole tick (Map::ClassifyForFanOut) - so one object's
        // calls always land in the SAME shard's slot, where push order is chronological order.
        // Nothing here depends on relative ordering between DIFFERENT objects' entries (only
        // set membership is read back), which is the part shard-major replay does NOT preserve.
        void AddUpdateObject(Object* obj)
        {
            if (Optional<uint32> shard = CurrentFanOutShardForThisMap())
                _updateObjectsBuffer.Push(*shard, { obj, true });
            else
                _updateObjects.insert(obj);
        }

        void RemoveUpdateObject(Object* obj)
        {
            if (Optional<uint32> shard = CurrentFanOutShardForThisMap())
                _updateObjectsBuffer.Push(*shard, { obj, false });
            else
                _updateObjects.erase(obj);
        }

        size_t GetActiveNonPlayersCount() const
        {
            return m_activeNonPlayers.size();
        }

        virtual std::string GetDebugInfo() const;

    private:

        void SetTimer(uint32 t) { i_gridExpiry = t < MIN_GRID_DELAY ? MIN_GRID_DELAY : t; }

        void SendInitSelf(Player* player);

        template <typename T>
        bool MapObjectCellRelocation(T* object, Cell new_cell, char const* objType);

        // Boundary-crossing detection for Phase 4 (ARGUSCORE_FIXES.md) - called from both
        // DiffGrid-success branches of MapObjectCellRelocation, right after the existing
        // grid-cell relocation (RemoveFromGrid/AddToGrid) already above it. Deliberately compares
        // the DESTINATION GRID's partition (new_cell.GridX()/GridY(), via GetPartitionIndexForGrid)
        // against ShardOf(object) - the object's current bookkeeping shard - not the source grid,
        // so this interoperates correctly if a prior combat-pin transfer already moved the
        // object's bookkeeping elsewhere. Only enqueues a deferred transfer (drained at the
        // Map::DelayedUpdate barrier, never mutates _objectsStore/_objectShardIndex here) - this
        // function runs mid-Map::Update(), before the MapManager-wide join barrier, so it must
        // never do the actual transfer synchronously (see HandleCrossPartitionTransfer's own
        // comment for why). GetShardCount()<=1 fast path first - zero cost on an unpartitioned
        // map, matching every other guard this feature has added. Defined in Map.cpp, not inline
        // here, same reason as TransferWorldObjectToPartition/ResolveCrossPartitionPair - the
        // body's non-dependent ObjectAccessor::GetWorldObject call needs ObjectAccessor.h, which
        // Map.h deliberately doesn't include.
        template <typename T>
        void EnqueueCrossPartitionTransferIfNeeded(T* object, Cell const& new_cell);

        bool CreatureCellRelocation(Creature* creature, Cell new_cell);
        bool GameObjectCellRelocation(GameObject* go, Cell new_cell);
        bool DynamicObjectCellRelocation(DynamicObject* go, Cell new_cell);
        bool AreaTriggerCellRelocation(AreaTrigger* at, Cell new_cell);

        template<class T> void InitializeObject(T* obj);
        void AddCreatureToMoveList(Creature* c, float x, float y, float z, float ang);
        void RemoveCreatureFromMoveList(Creature* c);
        void AddGameObjectToMoveList(GameObject* go, float x, float y, float z, float ang);
        void RemoveGameObjectFromMoveList(GameObject* go);
        void AddDynamicObjectToMoveList(DynamicObject* go, float x, float y, float z, float ang);
        void RemoveDynamicObjectFromMoveList(DynamicObject* go);
        void AddAreaTriggerToMoveList(AreaTrigger* at, float x, float y, float z, float ang);
        void RemoveAreaTriggerFromMoveList(AreaTrigger* at);

        bool _creatureToMoveLock;
        std::vector<Creature*> _creaturesToMove;
        // Phase 3 redesign, Stage 2 (ARGUSCORE_FIXES.md) - see Map::AddCreatureToMoveList's own
        // comment. _moveState is per-object and already safe under interior-classification
        // disjointness - only the shared vector's push_back needs buffering.
        PerShardDeferredBuffer<Creature*> _creaturesToMoveBuffer;

        bool _gameObjectsToMoveLock;
        std::vector<GameObject*> _gameObjectsToMove;
        PerShardDeferredBuffer<GameObject*> _gameObjectsToMoveBuffer;

        bool _dynamicObjectsToMoveLock;
        // Pre-existing, unrelated to Phase 3: MoveAllDynamicObjectsInMoveList has no call site
        // anywhere in src/, so nothing ever drains _dynamicObjectsToMove - it grows unbounded.
        // _dynamicObjectsToMoveBuffer below feeds this same dead container; it is real, tested
        // plumbing (kept symmetric with the other three move lists), not a sign the container
        // is live.
        std::vector<DynamicObject*> _dynamicObjectsToMove;
        PerShardDeferredBuffer<DynamicObject*> _dynamicObjectsToMoveBuffer;

        bool _areaTriggersToMoveLock;
        std::vector<AreaTrigger*> _areaTriggersToMove;
        PerShardDeferredBuffer<AreaTrigger*> _areaTriggersToMoveBuffer;

        bool IsGridLoaded(GridCoord const&) const;
        void EnsureGridCreated(GridCoord const&);
        bool EnsureGridLoaded(Cell const&);
        void EnsureGridLoadedForActiveObject(Cell const&, WorldObject const* object);

        void buildNGridLinkage(NGridType* pNGridType) { pNGridType->link(this); }

        NGridType* getNGrid(uint32 x, uint32 y) const
        {
            ASSERT(x < MAX_NUMBER_OF_GRIDS && y < MAX_NUMBER_OF_GRIDS, "x = %u, y = %u", x, y);
            return i_grids[x][y];
        }

        bool isGridObjectDataLoaded(uint32 x, uint32 y) const { return getNGrid(x, y)->isGridObjectDataLoaded(); }
        void setGridObjectDataLoaded(bool pLoaded, uint32 x, uint32 y) { getNGrid(x, y)->setGridObjectDataLoaded(pLoaded); }

        void setNGrid(NGridType* grid, uint32 x, uint32 y);
        void ScriptsProcess();

        void SendObjectUpdates();

    protected:
        virtual void LoadGridObjects(NGridType* grid, Cell const& cell);

        MapEntry const* i_mapEntry;
        Difficulty i_spawnMode;
        uint32 i_InstanceId;
        uint32 m_worldLayer;    // 0 for non-layered maps; >0 for open-world layer copies
        Trinity::unique_weak_ptr<Map> m_weakRef;
        uint32 m_unloadTimer;
        float m_VisibleDistance;
        // Phase 3 redesign (ARGUSCORE_FIXES.md) - Map-wide, NOT sharded. GameObject::AddToWorld's
        // InsertGameObjectModel call and EnsureGridLoaded's Balance() both write here from
        // whichever shard's worker thread happens to trigger them; isInLineOfSight/
        // getObjectHitPos/GetGameObjectFloor read it from ordinary LoS/trajectory/height checks,
        // just as reachable. AddToMap's "defer if the destination grid doesn't exist yet" check
        // (see that function's own comment) closes the Balance()-on-grid-creation case, but
        // ordinary GameObject creation on an ALREADY-loaded grid still reaches
        // InsertGameObjectModel from any worker thread. Stage 4 (ARGUSCORE_FIXES.md): read-heavy/
        // write-rare, so a shared_mutex rather than sharding or deferral - _dynamicTreeLock's own
        // comment on the accessors below has the read/write split.
        mutable std::shared_mutex _dynamicTreeLock;
        DynamicMapTree _dynamicTree;

        MapRefManager m_mapRefManager;
        MapRefManager::iterator m_mapRefIter;

        int32 m_VisibilityNotifyPeriod;

        using ActiveNonPlayers = std::set<WorldObject*>;
        ActiveNonPlayers m_activeNonPlayers;
        ActiveNonPlayers::iterator m_activeNonPlayersIter;

        // Objects that must update even in inactive grids without activating them
        using TransportsContainer = std::set<Transport*>;
        TransportsContainer _transports;
        TransportsContainer::iterator _transportsUpdateIter;

    private:
        Player* _GetScriptPlayerSourceOrTarget(Object* source, Object* target, ScriptInfo const* scriptInfo) const;
        Creature* _GetScriptCreatureSourceOrTarget(Object* source, Object* target, ScriptInfo const* scriptInfo, bool bReverse = false) const;
        GameObject* _GetScriptGameObjectSourceOrTarget(Object* source, Object* target, ScriptInfo const* scriptInfo, bool bReverse = false) const;
        Unit* _GetScriptUnit(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const;
        Player* _GetScriptPlayer(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const;
        Creature* _GetScriptCreature(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const;
        WorldObject* _GetScriptWorldObject(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const;
        void _ScriptProcessDoor(Object* source, Object* target, ScriptInfo const* scriptInfo) const;
        GameObject* _FindGameObject(WorldObject* pWorldObject, ObjectGuid::LowType guid) const;

        time_t i_gridExpiry;

        std::shared_ptr<TerrainInfo> m_terrain;
        uint16 m_forceEnabledNavMeshFilterFlags;
        uint16 m_forceDisabledNavMeshFilterFlags;

        NGridType* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];

        // Phase 3 (ARGUSCORE_FIXES.md) - one bitset per interior shard, plus one dedicated
        // trailing slot (index == GetShardCount()) used exclusively by the serial boundary pass.
        // Always sized shardCount+1, even when shardCount==1 (the extra bitset goes permanently
        // unused on unpartitioned maps - trivial memory, avoids conditional sizing logic
        // elsewhere). Sized in the constructor alongside _respawnTimes/_objectsStore's own
        // shardCount-driven resize.
        std::vector<std::bitset<TOTAL_NUMBER_OF_CELLS_PER_MAP*TOTAL_NUMBER_OF_CELLS_PER_MAP>> _markedCellsByShard;

        // Phase 3 (ARGUSCORE_FIXES.md) - this Map's private fan-out thread pool. Null until the
        // first tick that actually fans out (GetShardCount()>1, not force-disabled, population >=
        // Map.Partitioning.MinPopulationForFanout) - constructed lazily, never in the Map
        // constructor, so an unpartitioned or low-population partitioned map never pays for
        // threads it doesn't use. Once built it is kept even if population later dips back below
        // threshold (avoids thread create/join churn on population fluctuation); Map::Update()
        // just stops submitting to it.
        std::unique_ptr<PartitionWorkerPool> _partitionWorkerPool;

        // Phase 3 redesign (ARGUSCORE_FIXES.md) - "which shard's fan-out task is this OS thread
        // currently executing", set only for the duration of a PartitionWorkerPool task (via the
        // FanOutShardScope RAII guard wrapped around each Submit lambda body in Map::Update()) and
        // unset (Optional empty) everywhere else - the main thread, the serial boundary pass,
        // DelayedUpdate, and every unpartitioned map. thread_local rather than a parameter threaded
        // through the call graph because the code that needs this ambient context
        // (Map::AddToMap<T>, primarily) is reached from hundreds of SummonCreature/SummonGameObject/
        // spell-effect/AI call sites across the whole scripts tree, not a handful of funnel points -
        // mirrors this codebase's existing thread_local precedent (the RNG engine,
        // src/common/Utilities/Random.cpp) rather than inventing a new kind of ambient-state idiom.
        // Deliberately a static member (one instance per OS thread, shared across every Map, exactly
        // like the RNG precedent) rather than per-Map state - a given OS thread only ever executes
        // one PartitionWorkerPool task at a time regardless of which Map owns that pool.
        //
        // Carries the owning Map* alongside the shard index, not just the shard index alone - an
        // independent review found that a bare shard index is meaningless (and silently wrong) if
        // anything reachable from a fan-out task ever calls AddToMap on a DIFFERENT Map instance
        // (e.g. a cross-map summon or teleport helper) - comparing that other Map's computed
        // destination shard against THIS Map's shard index would compare two unrelated partition
        // layouts. AddToMap checks `this == fanOutContext->first` before trusting the shard value
        // at all, so a cross-Map call safely falls through to "not currently fanning out for this
        // Map" instead of an incorrect same/different-shard verdict either way.
        struct FanOutContext
        {
            Map* OwningMap;
            uint32 Shard;
        };
        static thread_local Optional<FanOutContext> t_currentFanOutContext;

        // RAII guard for t_currentFanOutContext - wrapped around each PartitionWorkerPool task's
        // body in Map::Update() so the flag is always cleared on the way out, including when the
        // task throws (caught by that same task's own try/catch) - a manual reset placed after the
        // catch block would still run in that case too, but RAII is the obviously-correct shape
        // and costs nothing extra. A private nested class (not a free struct in Map.cpp) so it can
        // reach the private static member above - nested classes have access to their enclosing
        // class's private members since C++11.
        struct FanOutShardScope
        {
            FanOutShardScope(Map* owningMap, uint32 shard) { t_currentFanOutContext = FanOutContext{ owningMap, shard }; }
            ~FanOutShardScope() { t_currentFanOutContext.reset(); }
            FanOutShardScope(FanOutShardScope const&) = delete;
            FanOutShardScope& operator=(FanOutShardScope const&) = delete;
        };

        // Phase 3 redesign, Stage 2 (ARGUSCORE_FIXES.md) - merges every PerShardDeferredBuffer
        // into its real container; see Map::Update()'s own call site comment for exactly when
        // this runs and why. Defined in Map.cpp.
        void DrainDeferredBuffers();

        //these functions used to process player/mob aggro reactions and
        //visibility calculations. Highly optimized for massive calculations
        void ProcessRelocationNotifies(const uint32 diff);

        bool i_scriptLock;
        std::set<WorldObject*> i_objectsToRemove;
        // Phase 3 redesign, Stage 2 fix (ARGUSCORE_FIXES.md, review finding) - buffers only the
        // final i_objectsToRemove.insert(); AddObjectToRemoveList's own comment explains why the
        // SetDestroyedObject/CleanupsBeforeDelete work ahead of it stays synchronous and what
        // that does/doesn't close.
        PerShardDeferredBuffer<WorldObject*> _objectsToRemoveBuffer;
        std::map<WorldObject*, bool> i_objectsToSwitch;
        // Phase 3 redesign, Stage 2 (ARGUSCORE_FIXES.md) - see AddObjectToSwitchList's own comment.
        PerShardDeferredBuffer<std::pair<WorldObject*, bool>> _objectsToSwitchBuffer;
        void ApplyObjectToSwitchList(WorldObject* obj, bool on);
        std::set<WorldObject*> i_worldObjects;
        // Phase 3 redesign, Stage 2 (ARGUSCORE_FIXES.md) - see AddWorldObject's own comment.
        PerShardDeferredBuffer<std::pair<WorldObject*, bool>> _worldObjectsBuffer;

        using ScriptScheduleMap = std::multimap<time_t, ScriptAction>;
        ScriptScheduleMap m_scriptSchedule;

    public:
        void ProcessRespawns();
        void ApplyDynamicModeRespawnScaling(WorldObject const* obj, ObjectGuid::LowType spawnId, uint32& respawnDelay, uint32 mode) const;

    private:
        // if return value is true, we can respawn
        // if return value is false, reschedule the respawn to new value of info->respawnTime iff nonzero, delete otherwise
        // if return value is false and info->respawnTime is nonzero, it is guaranteed to be greater than time(nullptr)
        bool CheckRespawn(RespawnInfo* info);
        void DoRespawn(SpawnObjectType type, ObjectGuid::LowType spawnId, uint32 gridId);
        bool AddRespawnInfo(RespawnInfo const& info);
        void UnloadAllRespawnInfos();
        RespawnInfo* GetRespawnInfo(SpawnObjectType type, ObjectGuid::LowType spawnId) const;
        void Respawn(RespawnInfo* info, CharacterDatabaseTransaction dbTrans = nullptr);
        void DeleteRespawnInfo(RespawnInfo* info, CharacterDatabaseTransaction dbTrans = nullptr);
        void DeleteRespawnInfoFromDB(SpawnObjectType type, ObjectGuid::LowType spawnId, CharacterDatabaseTransaction dbTrans = nullptr);

    public:
        void GetRespawnInfo(std::vector<RespawnInfo const*>& respawnData, SpawnObjectTypeMask types) const;
        void Respawn(SpawnObjectType type, ObjectGuid::LowType spawnId, CharacterDatabaseTransaction dbTrans = nullptr)
        {
            if (RespawnInfo* info = GetRespawnInfo(type, spawnId))
                Respawn(info, dbTrans);
        }
        void RemoveRespawnTime(SpawnObjectType type, ObjectGuid::LowType spawnId, CharacterDatabaseTransaction dbTrans = nullptr, bool alwaysDeleteFromDB = false)
        {
            if (RespawnInfo* info = GetRespawnInfo(type, spawnId))
                DeleteRespawnInfo(info, dbTrans);
            // Some callers might need to make sure the database doesn't contain any respawn time
            else if (alwaysDeleteFromDB)
                DeleteRespawnInfoFromDB(type, spawnId, dbTrans);
        }
        size_t DespawnAll(SpawnObjectType type, ObjectGuid::LowType spawnId);

        bool ShouldBeSpawnedOnGridLoad(SpawnObjectType type, ObjectGuid::LowType spawnId) const;
        template <typename T> bool ShouldBeSpawnedOnGridLoad(ObjectGuid::LowType spawnId) const { return ShouldBeSpawnedOnGridLoad(SpawnData::TypeFor<T>, spawnId); }

        SpawnGroupTemplateData const* GetSpawnGroupData(uint32 groupId) const;

        bool IsSpawnGroupActive(uint32 groupId) const;

        // Enable the spawn group, which causes all creatures in it to respawn (unless they have a respawn timer)
        // The force flag can be used to force spawning additional copies even if old copies are still around from a previous spawn
        bool SpawnGroupSpawn(uint32 groupId, bool ignoreRespawn = false, bool force = false, std::vector<WorldObject*>* spawnedObjects = nullptr);

        // Despawn all creatures in the spawn group if spawned, optionally delete their respawn timer, and disable the group
        bool SpawnGroupDespawn(uint32 groupId, bool deleteRespawnTimes = false, size_t* count = nullptr);

        // Disable the spawn group, which prevents any creatures in the group from respawning until re-enabled
        // This will not affect any already-present creatures in the group
        void SetSpawnGroupInactive(uint32 groupId) { SetSpawnGroupActive(groupId, false); }

        SpawnedPoolData& GetPoolData() { return *_poolData; }
        SpawnedPoolData const& GetPoolData() const { return *_poolData; }

        using FarSpellCallback = std::function<void(Map*)>;
        void AddFarSpellCallback(FarSpellCallback&& callback);

        // Phase 3 redesign, Stage 2 (ARGUSCORE_FIXES.md) - centralizes the "am I on a
        // PartitionWorkerPool worker thread currently fanning out for THIS Map" check every
        // PerShardDeferredBuffer call site needs, matching AddToMap's own OwningMap == this
        // pattern (see t_currentFanOutContext's own comment for why the Map-identity check
        // matters, not just the shard index). Public (moved here from a private section in Stage
        // 3, ARGUSCORE_FIXES.md review finding) - the guard half of the "guard + defer" pattern
        // needs to be callable everywhere AddFarSpellCallback (the defer half) already is, e.g.
        // Unit::SetVignette/GameObject::SetVignette.
        Optional<uint32> CurrentFanOutShardForThisMap() const
        {
            if (t_currentFanOutContext && t_currentFanOutContext->OwningMap == this)
                return t_currentFanOutContext->Shard;
            return {};
        }

        // Stage 6 (ARGUSCORE_FIXES.md) - map-agnostic sibling of CurrentFanOutShardForThisMap()
        // (deliberately without the per-instance OwningMap==this comparison, since t_currentFanOutContext
        // is one thread_local shared across every Map - see its own comment) for callers that
        // aren't tied to any one Map instance, e.g. GameEventMgr's debug-mode tripwire (a global,
        // timer-driven system that should never legitimately run from a live fan-out worker
        // thread, regardless of which Map that thread happens to be fanning out for right now).
        static bool IsAnyFanOutInProgressOnThisThread()
        {
            return t_currentFanOutContext.has_value();
        }

        void InitSpawnGroupState();
        void UpdateSpawnGroupConditions();

    private:
        // Type specific code for add/remove to/from grid
        template<class T>
        void AddToGrid(T* object, Cell const& cell);

        template<class T>
        void DeleteFromWorld(T*);

        // Sharded by static spawn-point partition assignment, same reasoning/rules as the
        // by-spawnId stores declared in the public section above - see ARGUSCORE_FIXES.md.
        // One shard (index 0) for an unpartitioned map, byte-for-byte equivalent to the single
        // flat container/heap this replaces.
        std::vector<std::unique_ptr<RespawnListContainer>> _respawnTimes;
        std::vector<RespawnInfoMap> _creatureRespawnTimesBySpawnId;
        std::vector<RespawnInfoMap> _gameObjectRespawnTimesBySpawnId;

        // This Map's OWN independent copy of its partition layout, taken once at construction
        // from sMapPartitionMgr and never touched again - NOT a pointer into the live
        // MapPartitionMgr singleton. This is load-bearing, not a style choice: independent
        // review found that MapPartitionMgr::Configure() (called from World::LoadConfigSettings,
        // reachable live via the .reload config GM command) does `_layouts.clear()` before
        // rebuilding - meaning ANY raw MapPartitionLayout const* obtained from
        // sMapPartitionMgr->GetLayout() before a reload goes dangling after ANY reload, even one
        // that only touches a different mapId's config. Worse, even a valid-looking non-dangling
        // re-fetch would desync from GetShardCount() (below) the moment an operator grows a
        // partition count or newly enables a map that already has live instances - the
        // per-shard vectors sized at construction stay their original size forever, while a
        // live re-query of the layout could return an index past their end -
        // GetPartitionIndexForPosition/GetPartitionIndexForSpawnId indexing straight into those
        // vectors with operator[] and no bounds check would then be a real out-of-bounds access.
        // A plain value (via Optional, not a pointer) sidesteps both problems at once: this Map
        // simply never looks at sMapPartitionMgr again after construction, so nothing it does
        // can ever go stale or fall out of bounds regardless of what any later reload changes -
        // genuinely realizing Decision 2 ("static layout, never re-drawn at runtime") rather
        // than just asserting it in a comment while the code quietly re-queried live state.
        Optional<MapPartitionLayout> _cachedPartitionLayout;

        // This Map's own copy of Map.Partitioning.HaloWidth, taken once at construction alongside
        // _cachedPartitionLayout (Phase 6, ARGUSCORE_FIXES.md - "Double-buffered halo snapshots").
        // Independent review caught that PublishHaloSnapshots() originally queried
        // sMapPartitionMgr->GetHaloWidth() live every tick instead of caching it here like the
        // layout above - functionally harmless today (nothing sizes a container off this value,
        // unlike the layout/shard-count desync _cachedPartitionLayout itself was added to
        // prevent), but a real inconsistency with the "frozen at construction, never re-drawn at
        // runtime" invariant every other piece of per-Map partition state in this design commits
        // to. Cached here for consistency, not because a live read was ever provably unsafe.
        float _cachedHaloWidth = 0.0f;

        // Phase 3 redesign (ARGUSCORE_FIXES.md) - the classification probe width
        // Map::ClassifyForFanOut uses to decide "interior" vs "boundary", cached at construction
        // for the same "frozen, never re-drawn at runtime" reasoning as _cachedHaloWidth above.
        // Deliberately NOT obj->GetGridActivationRange() (the original Phase 3 attempt's choice,
        // confirmed by an independent review to be unsound - GetGridActivationRange can be as
        // little as 50y for a non-active creature, while ordinary Update()-reachable code
        // (WorldObject::UpdateObjectVisibility, AI/spell target searches) reads out to
        // GetVisibilityRange(), up to MAX_VISIBILITY_DISTANCE). Set to
        // MAX_VISIBILITY_DISTANCE + a small margin so the boundary check is strict, not an exact
        // tie at the float level.
        //
        // What this actually proves, precisely (a second independent review corrected an
        // over-broad version of this claim): Cell::Visit hard-clamps every cell-based search
        // radius to SIZE_OF_GRIDS (== MAX_VISIBILITY_DISTANCE, CellImpl.h), so a classified
        // object's OWN CellArea (Map::VisitNearbyCellsOf) can never reach outside its own shard's
        // grid range once this probe classifies it "interior" - that is what guarantees no object
        // is ever Update()-dispatched by two shards at once, the property the interior/boundary
        // split exists to prove. It does NOT mean every read reachable from that object's Update()
        // is confined the same way: an object sitting at the far edge of the anchor's cell area
        // can itself be near a partition boundary and run its OWN visibility/AI searches (up to
        // its own MAX_VISIBILITY_DISTANCE) across it while a neighboring shard concurrently ticks
        // objects on the other side. That residual cross-shard read exposure is exactly what
        // Phase 5's combat/threat/aura guards (IsCrossPartition + defer-to-barrier) and Phase 6's
        // halo snapshots exist to cover - this probe width is necessary for the dispatch proof,
        // not a substitute for those.
        float _cachedClassificationProbeWidth = 0.0f;

        // Phase 3 redesign (ARGUSCORE_FIXES.md) - Map.Partitioning.MinPopulationForFanout, cached
        // at construction for the same reasoning as _cachedHaloWidth above. An independent review
        // found Map::Update() was re-reading sMapPartitionMgr->GetMinPopulationForFanout() live
        // every tick - a live .reload config could arm real concurrency on an already-running map
        // mid-session, the exact class of bug _cachedHaloWidth was already added once to prevent
        // for a different config value.
        uint32 _cachedMinPopulationForFanout = 0;

        // Phase 3 redesign (ARGUSCORE_FIXES.md) - the Phase 7 kill-switch's value for THIS tick,
        // latched once at the top of Map::Update() and read by Map::IsCrossPartition instead of a
        // live sMapPartitionMgr->IsForceDisabled(GetId()) call - see IsCrossPartition's own
        // comment for why a live read there was unsafe once a real fan-out existed (toggling the
        // switch mid-tick could make already-in-flight worker-thread guards go inert for the rest
        // of that tick). Deliberately NOT applied to IsNearPartitionBoundary/PublishHaloSnapshots/
        // EnqueueCrossPartitionTransferIfNeeded/the PlayerRelocation vehicle hook - none of those
        // are reachable from a PartitionWorkerPool worker thread during the parallel phase (they
        // only ever run on the main thread or serially post-join), so they keep reading live and
        // stay maximally responsive to a toggle, which is strictly better for them.
        bool _tickForceDisabled = false;

        // Stage 8 follow-up fix (ARGUSCORE_FIXES.md, closing the Stage 7 recheck's "Known,
        // NOT-yet-fixed gap" in IsCrossPartition) - records THIS tick's actual ClassifyForFanOut
        // dispatch decision for every object classified this tick (interior shard index, or
        // GetShardCount() as the sentinel for "boundary bucket, processed serially" - same
        // sentinel convention used elsewhere for per-shard buffer array sizing). Populated
        // entirely during the serial classification loop at the top of the fan-out branch of
        // Map::Update(), BEFORE PartitionWorkerPool::Submit() launches any worker task for this
        // tick, and never mutated again until the next tick's classification loop starts (which
        // itself only ever runs on the main thread, after every previous tick's worker tasks have
        // already been joined via WaitAll()) - so every worker-thread read of this map during the
        // parallel phase (via Map::IsCrossPartition) sees a fully-built, read-only snapshot with
        // no synchronization needed. This is the precise "which worker thread is CURRENTLY
        // processing each side THIS tick" answer ShardOf/GetPartitionIndexForObject alone
        // couldn't provide (see IsCrossPartition's own comment) - keyed by raw WorldObject*
        // (valid for the object's whole classify-then-dispatch-then-join lifetime within a single
        // tick; nothing is deleted mid-Update(), only later in the serial cleanup phase) rather
        // than ObjectGuid, avoiding a redundant GUID hash on the hot classification path. Cleared
        // at the top of every Map::Update() call (both fan-out and non-fan-out branches), not just
        // the fan-out one, so stale entries from an actual-fan-out tick can never be misread on a
        // later non-fan-out tick.
        std::unordered_map<WorldObject const*, uint32> _tickDispatchShard;

        // Lazy shard cache for GetPartitionIndexForSpawnId - see that method's own comment for
        // why this exists (DeleteFromDB's spawn-data-deleted-before-instance-removed race).
        // Split by type, matching _creatureRespawnTimesBySpawnId/_gameObjectRespawnTimesBySpawnId
        // - creature and gameobject spawnIds are separate ID namespaces, so a single shared
        // cache would risk conflating an unrelated pair that happen to share a numeric spawnId.
        //
        // Phase 3 redesign, Stage 4 (ARGUSCORE_FIXES.md) - genuinely unbounded (every spawnId on
        // the whole continent, potentially tens of thousands), so eager pre-population at
        // construction (the shape used for the small, fixed-size _guidGenerators below) would be
        // real overreach here - stays lazy, guarded by a shared_mutex instead: shared_lock for the
        // overwhelmingly common already-cached read, unique_lock only on first-touch insert.
        // std::unordered_map has no reference/iterator stability guarantee across rehashing, so
        // (unlike _guidGenerators' std::map) GetPartitionIndexForSpawnId returns the shard value
        // by copy while still under the lock - never a reference into the map.
        mutable std::shared_mutex _creatureSpawnIdShardCacheLock;
        mutable std::unordered_map<ObjectGuid::LowType, uint32> _creatureSpawnIdShardCache;
        mutable std::shared_mutex _gameObjectSpawnIdShardCacheLock;
        mutable std::unordered_map<ObjectGuid::LowType, uint32> _gameObjectSpawnIdShardCache;
        mutable std::shared_mutex _areaTriggerSpawnIdShardCacheLock;
        mutable std::unordered_map<ObjectGuid::LowType, uint32> _areaTriggerSpawnIdShardCache;

        // Number of respawn/spawnId shards this Map was constructed with - 1 for an
        // unpartitioned map, or the opted-in mapId's configured partition count otherwise.
        // Computed once at construction (Decision 2 - static layout, never re-drawn at
        // runtime), not re-queried from MapPartitionMgr on every call.
        uint32 GetShardCount() const { return uint32(_respawnTimes.size()); }

        // Pure position -> shard lookup, shared by both Phase 1 (spawnId-derived, static
        // position) and Phase 2 (_objectsStore, live position) sharding. 0 immediately on an
        // unpartitioned map, without even a grid-coord computation - the common-case fast path.
        uint32 GetPartitionIndexForPosition(float x, float y) const;

        // Grid-coordinate-based partition lookup, factored out of GetPartitionIndexForPosition
        // (Phase 4, ARGUSCORE_FIXES.md) - the cross-partition transfer hook in
        // MapObjectCellRelocation already has a destination Cell's grid coordinates on hand
        // before the object's own Relocate() has run, so it calls this directly instead of
        // round-tripping through a float position that isn't authoritative yet. Same "0
        // immediately on an unpartitioned map" fast path as GetPartitionIndexForPosition.
        uint32 GetPartitionIndexForGrid(uint32 gridX, uint32 gridY) const;

        // Refreshes WorldObject::PublishHaloSnapshot() for every tracked object (and every
        // in-world player) currently within Map.Partitioning.HaloWidth of a partition boundary -
        // the write side of Phase 6 (ARGUSCORE_FIXES.md). GetShardCount()<=1 fast path first, so
        // an unpartitioned map never walks _objectsStore/players for this at all, not just cheaply
        // per-object. Called exactly once, from Map::DelayedUpdate, immediately after the
        // _farSpellCallbacks drain - see that call site's own comment for why that specific point
        // in the tick (not the top of DelayedUpdate, unlike Phase 4/5's own barrier-phase hooks)
        // is the correct one: it's the last point anything can still write an object's position
        // this tick, including _farSpellCallbacks' own real callback producers.
        void PublishHaloSnapshots();

        // Ordinary position-based cross-partition transfer (Phase 4, ARGUSCORE_FIXES.md) -
        // called from the deferred callbacks queued by the boundary-crossing hook in
        // MapObjectCellRelocation and the player-vehicle hook in PlayerRelocation, always at the
        // Map::DelayedUpdate barrier, never mid-tick. Before moving obj itself, checks whether it
        // has a live combat/threat reference (Unit::GetCombatManager().HasCombat()) and if so
        // drags every current reference partner onto the same targetShard too - the "Piece 3"
        // dependency Phase 5 already documented: refusing the move instead would leave
        // _objectShardIndex mismatched from the object's actual position indefinitely (nothing
        // re-evaluates a refused transfer until the object crosses another grid), silently
        // breaking Phase 5's "both sides of a reference are always same-partition" invariant the
        // first time either side walks. Reuses TransferWorldObjectToPartition, which already
        // knows how to drag controlled pets and vehicle passengers along for whichever side
        // actually moves.
        void HandleCrossPartitionTransfer(WorldObject* obj, uint32 targetShard);

        // Which shard a given spawnId's respawn/by-spawnId-store data lives in. Computed once
        // per spawnId from that spawn's static home position (sObjectMgr spawn data) and then
        // CACHED (_creatureSpawnIdShardCache/_gameObjectSpawnIdShardCache, both mutable - a
        // const-correct lazy cache) rather than recomputed on every call. This isn't just a
        // performance optimization: Creature::DeleteFromDB/GameObject::DeleteFromDB delete the
        // spawn's sObjectMgr entry synchronously, then defer the actual instance removal (via
        // AddObjectToRemoveList) to a later map-update pass - so by the time
        // RemoveCreatureFromSpawnIdStore/RemoveGameObjectFromSpawnIdStore actually runs and
        // needs this spawnId's shard again, sObjectMgr->GetSpawnData() for it may already
        // return nullptr. Recomputing at that point would silently fall back to shard 0 and
        // erase from the wrong shard, permanently orphaning a soon-to-be-deleted pointer in
        // whatever shard it was really inserted into - caching the value computed at insert
        // time (while spawn data still existed) means removal reuses that same value instead
        // of re-deriving it from state that's no longer guaranteed to exist. Same reasoning
        // Phase 2 already applied to _objectShardIndex, extended to Phase 1's containers.
        // Returns (and, if the layout lookup itself short-circuits, never caches) 0 for an
        // unpartitioned map without even querying spawn data - the common-case fast path.
        //
        // Known, honest scope boundary (independent review, not yet fixed): this cache is never
        // invalidated if a spawn's STATIC home position is edited in place for an existing
        // spawnId while this Map instance is alive - e.g. Creature::SaveToDB after a `.npc move`
        // (Creature.cpp, `data.spawnPoint.Relocate(this)`) rewrites sObjectMgr's SpawnData for
        // that spawnId without this Map ever being told. If that move crosses a partition
        // boundary, this cache keeps returning the stale (pre-move) shard for the rest of this
        // Map object's lifetime - the by-spawnId store entry itself stays internally consistent
        // (insert/remove/lookup still all agree with each other, so no crash/corruption), it's
        // just bookkept under the wrong partition until the map is fully unloaded and reloaded. A
        // full fix needs a genuine spawnId-shard-transfer mechanism (mirroring Phase 4's
        // TransferObjectToPartition, but for _creatureBySpawnIdStore/_gameobjectBySpawnIdStore
        // specifically) hooked into Creature::SaveToDB/GameObject::SaveToDB - not built here,
        // since this path is currently 100% inert (Map.Partitioning.Enabled defaults to 0, and
        // even when enabled this only matters for a spawn deliberately moved across a configured
        // partition boundary via a GM command, not anything a player can trigger).
        uint32 GetPartitionIndexForSpawnId(SpawnObjectType type, ObjectGuid::LowType spawnId) const;

        RespawnInfoMap* GetRespawnMapForType(SpawnObjectType type, uint32 shard)
        {
            switch (type)
            {
                default:
                    ABORT();
                case SPAWN_TYPE_CREATURE:
                    return &_creatureRespawnTimesBySpawnId[shard];
                case SPAWN_TYPE_GAMEOBJECT:
                    return &_gameObjectRespawnTimesBySpawnId[shard];
                case SPAWN_TYPE_AREATRIGGER:
                    return nullptr;
            }
        }
        RespawnInfoMap const* GetRespawnMapForType(SpawnObjectType type, uint32 shard) const
        {
            switch (type)
            {
                default:
                    ABORT();
                case SPAWN_TYPE_CREATURE:
                    return &_creatureRespawnTimesBySpawnId[shard];
                case SPAWN_TYPE_GAMEOBJECT:
                    return &_gameObjectRespawnTimesBySpawnId[shard];
                case SPAWN_TYPE_AREATRIGGER:
                    return nullptr;
            }
        }

        void SetSpawnGroupActive(uint32 groupId, bool state);
        std::unordered_set<uint32> _toggledSpawnGroupIds;

        uint32 _respawnCheckTimer;
        std::unordered_map<uint32, uint32> _zonePlayerCountMap;

        ZoneDynamicInfoMap _zoneDynamicInfo;
        IntervalTimer _weatherUpdateTimer;

        ObjectGuidGenerator& GetGuidSequenceGenerator(HighGuid high);

        // Phase 3 redesign, Stage 4 (ARGUSCORE_FIXES.md) - GetGuidSequenceGenerator's
        // try_emplace can mutate this map's tree on first use of a given HighGuid on this Map,
        // reachable from any fan-out worker thread doing ordinary object creation
        // (Creature/GameObject/AreaTrigger spawns, items, etc. all generate a low guid).
        // ObjectGuidGenerator::_nextGuid is already std::atomic, so once an entry exists,
        // concurrent Generate() calls on it need no lock at all - this guards only the map's own
        // insert. std::map (unlike std::unordered_map) never invalidates or moves existing
        // elements on insert, so GetGuidSequenceGenerator can safely return a reference used
        // outside the lock. shared_lock for the common already-exists read, unique_lock only for
        // first-touch insert - same double-checked shape as the spawn-id shard caches above.
        mutable std::shared_mutex _guidGeneratorsLock;
        std::map<HighGuid, ObjectGuidGenerator> _guidGenerators;
        std::unique_ptr<SpawnedPoolData> _poolData;
        std::vector<MapStoredObjectTypesContainer> _objectsStore;
        // guid -> shard it was actually inserted into (Phase 2 - see ARGUSCORE_FIXES.md).
        //
        // Phase 3 redesign (ARGUSCORE_FIXES.md) - _objectShardIndexLock guards THIS member only.
        // An independent review found that even after Stage 1's AddToMap defer fix (which stops a
        // worker thread from ever creating an object outside its own shard's territory), ordinary
        // SAME-shard object creation still writes here from whichever shard's worker thread
        // created the object - AddToObjectsStore/RemoveFromObjectsStore run on every summon/
        // despawn, not just the rare cross-shard case - while Map::ShardOf reads it from every
        // worker thread on every IsCrossPartition call (every guarded combat/threat/damage/aura
        // interaction). An unsynchronized concurrent unordered_map insert/erase-vs-find is a real
        // data race, and unlike most of this feature's other deferred-to-later-stages gaps, a
        // torn read here could return a garbage shard value and crash on the resulting
        // out-of-bounds _objectsStore[shard] access - severe enough, and central enough to this
        // stage's own ClassifyForFanOut/IsCrossPartition machinery, to fix now rather than defer
        // to Stage 4 with the rest of the §A "targeted locks" work.
        //
        // CLOSED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - this was flagged
        // here as "deliberately NOT covered... explicitly deferred to Stage 4", but Stage 4's own
        // actual 9-item scope (ARGUSCORE_FIXES.md) never included it - it fell through the cracks
        // between being named as deferred and actually being picked up. Confirmed genuinely,
        // pervasively reachable: shard S's worker thread calling GetCreature(guid) (via ordinary
        // ObjectAccessor::GetUnit/GetCreature/etc. lookups - ubiquitous throughout AI/spell/aura
        // code, not a narrow path) for an object actually living in shard T's container races
        // shard T's own concurrent AddToObjectsStore/RemoveFromObjectsStore for its own
        // spawns/despawns that same tick - an unsynchronized concurrent unordered_map find-vs-
        // insert/erase, real corruption/crash risk, not narrow or theoretical. Fixed with
        // _objectsStoreLock below, same shape as Stage 9's by-spawnId-store fix: a shared_mutex
        // covering the whole `_objectsStore` vector (not per-shard - concurrent reads are the hot,
        // common case; inserts/removes happen at spawn/despawn frequency, much rarer), shared_lock
        // for GetWorldObjectInObjectsStore's Find, unique_lock for AddToObjectsStore/
        // RemoveFromObjectsStore/TransferObjectToPartition's Insert/Remove. Deliberately a
        // SEPARATE lock from _objectShardIndexLock below (not one lock covering both) - every
        // existing caller already closes _objectShardIndexLock's own scope before ever touching
        // _objectsStore, so the two never need to be held simultaneously, and keeping them
        // separate avoids serializing the (already-locked, already-correct) shard-index bookkeeping
        // path against this new lock for no reason.
        mutable std::shared_mutex _objectsStoreLock;

        // Deliberately NOT also covering _objectsStore[shard]'s own per-type containers here -
        // that is a separate, still-open race (shard 0's worker thread calling GetCreature(guid)
        // for an object living in shard 1's container while shard 1 concurrently inserts into
        // that same container) explicitly deferred to Stage 4 alongside _dynamicTree/formations/
        // world states - see ARGUSCORE_FIXES.md's Stage 1 entry for the full list of what Stage 1
        // does and does not close. CORRECTED: see _objectsStoreLock's own comment above - actually
        // closed now, this comment's own "deferred to Stage 4" claim never actually happened.
        mutable std::shared_mutex _objectShardIndexLock;
        std::unordered_map<ObjectGuid, uint32> _objectShardIndex;

        // Stage 9 follow-up fix (ARGUSCORE_FIXES.md, independent review finding) - the sharded,
        // no-lock design below (Phase 1/Stage 4's "disjoint-shard-slot invariant") assumed each
        // shard's worker thread only ever touches its own _creatureBySpawnIdStore[shard] slot,
        // since GetPartitionIndexForSpawnId(type, spawnId) is computed here and
        // GetPartitionIndexForObject/GetPartitionIndexForGrid (this tick's actual dispatch shard)
        // are computed elsewhere - but nothing ever proved these two independently-computed shard
        // values always agree for a given object. They're based on DIFFERENT positions (this
        // spawnId's STATIC home position vs. the object's LIVE current position) and routinely
        // diverge for anything that has moved away from its spawn point without crossing back -
        // completely ordinary gameplay (wandering, chasing, fleeing, being dragged across a
        // boundary by CC/knockback), not a rare edge case. Confirmed reachable unguarded:
        // Creature::AddToWorld/RemoveFromWorld -> AddCreatureToSpawnIdStore/
        // RemoveCreatureFromSpawnIdStore (called only after Map::AddToMap's OWN guard already
        // confirmed the object's LIVE-position shard matches the calling thread - which says
        // nothing about whether its STATIC spawnId shard also does), and
        // Map::AddCreatureToFormation -> GetCreaturesBySpawnId, a synchronous read whose caller
        // uses the result immediately (can't be deferred the way a fire-and-forget write could).
        // Since the read side can't be deferred, fixed with a lock rather than trying to force
        // this into the defer/replay pattern used elsewhere - one shared_mutex per container
        // (covering every shard's slot, not per-shard, for simplicity; none of these three
        // containers are a per-tick-Update() hot path, only creature/gameobject/areatrigger
        // spawn/despawn and formation search) rather than the double-checked shared/unique split
        // used for the spawn-id shard CACHES above (those are genuinely hot, single-key lookups;
        // these are occasional multi-element inserts/erases/range-reads). The sharding itself is
        // left in place (still correct, still what GetPartitionIndexForSpawnId's caching exists to
        // support) - this only adds the synchronization the "disjoint slot" assumption turned out
        // to need.
        mutable std::shared_mutex _creatureBySpawnIdStoreLock;
        mutable std::shared_mutex _gameobjectBySpawnIdStoreLock;
        mutable std::shared_mutex _areaTriggerBySpawnIdStoreLock;
        std::vector<CreatureBySpawnIdContainer> _creatureBySpawnIdStore;
        std::vector<GameObjectBySpawnIdContainer> _gameobjectBySpawnIdStore;
        std::vector<AreaTriggerBySpawnIdContainer> _areaTriggerBySpawnIdStore;
        std::unordered_map<uint32/*cellId*/, std::unordered_set<Corpse*>> _corpsesByCell;
        std::unordered_map<ObjectGuid, Corpse*> _corpsesByPlayer;
        std::unordered_set<Corpse*> _corpseBones;

        std::unordered_set<Object*> _updateObjects;
        // Phase 3 redesign, Stage 2 (ARGUSCORE_FIXES.md) - see AddUpdateObject's own comment.
        PerShardDeferredBuffer<std::pair<Object*, bool>> _updateObjectsBuffer;

        MPSCQueue<FarSpellCallback> _farSpellCallbacks;

        /*********************************************************/
        /***                   Phasing                         ***/
        /*********************************************************/
    public:
        MultiPersonalPhaseTracker& GetMultiPersonalPhaseTracker() { return _multiPersonalPhaseTracker; }
        void UpdatePersonalPhasesForPlayer(Player const* player);

    private:
        MultiPersonalPhaseTracker _multiPersonalPhaseTracker;

        /*********************************************************/
        /***                   WorldStates                     ***/
        /*********************************************************/
    public:
        int32 GetWorldStateValue(int32 worldStateId) const;
        void SetWorldStateValue(int32 worldStateId, int32 value, bool hidden);
        // Phase 3 redesign, Stage 4 (ARGUSCORE_FIXES.md) - was `WorldStateValueContainer
        // const&`, letting WorldStateMgr::FillInitialWorldStates (reachable from a fan-out worker
        // thread via a player's own zone-change Update()) iterate the raw unordered_map while
        // SetWorldStateValue writes it from another shard - real iterator invalidation/corruption.
        // Returns a copy under the lock instead; this is a per-login/zone-change call, not a hot
        // per-tick path, so the copy is cheap relative to the correctness it buys.
        WorldStateValueContainer GetWorldStateValues() const
        {
            std::shared_lock<std::shared_mutex> lock(_worldStateValuesLock);
            return _worldStateValues;
        }

    private:
        // Phase 3 redesign, Stage 4 (ARGUSCORE_FIXES.md) - SetWorldStateValue is a
        // read-modify-write reachable from fan-out worker threads (boss/event AI calling
        // instance->SetWorldStateValue), and its value is read back in the same tick to gate
        // other logic (scripts routinely set then immediately branch on a world state) - ruling
        // out deferral, same reasoning as _zoneDynamicInfo's sibling A17 items but this one can't
        // just defer. A real lock, not sharding: world state IDs aren't spatial, so there's no
        // shard to key off.
        mutable std::shared_mutex _worldStateValuesLock;
        WorldStateValueContainer _worldStateValues;
        // Phase 3 redesign, Stage 4 fix (ARGUSCORE_FIXES.md, review finding) - see
        // SetWorldStateValue's own comment for why this is a separate lock from
        // _worldStateValuesLock above.
        mutable std::recursive_mutex _worldStateBroadcastLock;

        // Phase 3 redesign, Stage 4 (ARGUSCORE_FIXES.md) - see AddCreatureToFormation's own
        // comment above for why this moved private and gained a lock, and why it's held across
        // the whole add/remove sequence rather than just the container access. recursive_mutex
        // (changed from a plain mutex by the Stage 4 review fix): RemoveCreatureFromFormation
        // calls the target's ZoneScript::OnCreatureGroupDepleted while holding this lock, and that
        // callback is arbitrary boss/instance script code that could legitimately trigger another
        // creature spawn/despawn reaching back into AddCreatureToFormation/RemoveCreatureFromFormation
        // on the same thread - a plain mutex would self-deadlock on that path.
        mutable std::recursive_mutex _creatureGroupHolderLock;
        std::unordered_map<ObjectGuid::LowType /*leaderSpawnId*/, CreatureGroup*> _creatureGroupHolder;

        /*********************************************************/
        /***                   Vignettes                       ***/
        /*********************************************************/
    public:
        void AddInfiniteAOIVignette(Vignettes::VignetteData* vignette);
        void RemoveInfiniteAOIVignette(Vignettes::VignetteData* vignette);
        std::vector<Vignettes::VignetteData*> const& GetInfiniteAOIVignettes() const { return _infiniteAOIVignettes; }

    private:
        std::vector<Vignettes::VignetteData*> _infiniteAOIVignettes;
        PeriodicTimer _vignetteUpdateTimer;
};

enum class InstanceResetMethod : uint8
{
    Manual,
    OnChangeDifficulty,
    Expire
};

enum class InstanceResetResult : uint8
{
    Success,
    NotEmpty,
    CannotReset
};

class TC_GAME_API InstanceMap : public Map
{
    public:
        InstanceMap(uint32 id, time_t, uint32 InstanceId, Difficulty SpawnMode, TeamId InstanceTeam, InstanceLock* instanceLock,
            Optional<uint32> lfgDungeonsId);
        ~InstanceMap();
        bool AddPlayerToMap(Player* player, bool initPlayer = true) override;
        void RemovePlayerFromMap(Player*, bool) override;
        void Update(uint32) override;
        void CreateInstanceData();
        InstanceResetResult Reset(InstanceResetMethod method);
        uint32 GetScriptId() const { return i_script_id; }
        std::string const& GetScriptName() const;
        InstanceScript* GetInstanceScript() { return i_data; }
        InstanceScript const* GetInstanceScript() const { return i_data; }
        InstanceScenario* GetInstanceScenario() { return i_scenario.get(); }
        InstanceScenario const* GetInstanceScenario() const { return i_scenario.get(); }
        void SetInstanceScenario(InstanceScenario* scenario);
        InstanceLock const* GetInstanceLock() const { return i_instanceLock; }
        void UpdateInstanceLock(UpdateBossStateSaveDataEvent const& updateSaveDataEvent);
        void UpdateInstanceLock(UpdateAdditionalSaveDataEvent const& updateSaveDataEvent);
        void CreateInstanceLockForPlayer(Player* player);
        TransferAbortParams CannotEnter(Player* player) override;

        uint32 GetMaxPlayers() const;
        TeamId GetTeamIdInInstance() const;
        Team GetTeamInInstance() const { return GetTeamIdInInstance() == TEAM_ALLIANCE ? ALLIANCE : HORDE; }
        Optional<uint32> GetLfgDungeonsId() const { return i_lfgDungeonsId; }

        virtual void InitVisibilityDistance() override;

        Group* GetOwningGroup() const { return i_owningGroupRef.getTarget(); }
        void TrySetOwningGroup(Group* group);

        std::string GetDebugInfo() const override;
    private:
        Optional<SystemTimePoint> i_instanceExpireEvent;
        InstanceScript* i_data;
        uint32 i_script_id;
        std::unique_ptr<InstanceScenario> i_scenario;
        InstanceLock* i_instanceLock;
        GroupInstanceReference i_owningGroupRef;
        Optional<uint32> i_lfgDungeonsId;
};

class TC_GAME_API BattlegroundMap : public Map
{
    public:
        BattlegroundMap(uint32 id, time_t, uint32 InstanceId, Difficulty spawnMode);
        ~BattlegroundMap();

        bool AddPlayerToMap(Player* player, bool initPlayer = true) override;
        void RemovePlayerFromMap(Player*, bool) override;
        TransferAbortParams CannotEnter(Player* player) override;
        void SetUnload();
        //void UnloadAll(bool pForce);
        void RemoveAllPlayers() override;
        void Update(uint32 diff) override;

        virtual void InitVisibilityDistance() override;
        Battleground* GetBG() const { return m_bg; }
        void SetBG(Battleground* bg) { m_bg = bg; }

        uint32 GetScriptId() const { return _scriptId; }
        std::string const& GetScriptName() const;
        BattlegroundScript* GetBattlegroundScript() { return _battlegroundScript.get(); }
        BattlegroundScript const* GetBattlegroundScript() const { return _battlegroundScript.get(); }

        void InitScriptData();
    private:
        Battleground* m_bg;
        std::unique_ptr<BattlegroundScript> _battlegroundScript;
        uint32 _scriptId;
};

template<class T, class CONTAINER>
inline void Map::Visit(Cell const& cell, TypeContainerVisitor<T, CONTAINER>& visitor)
{
    const uint32 x = cell.GridX();
    const uint32 y = cell.GridY();
    const uint32 cell_x = cell.CellX();
    const uint32 cell_y = cell.CellY();

    if (!cell.NoCreate() || IsGridLoaded(GridCoord(x, y)))
    {
        EnsureGridLoaded(cell);
        getNGrid(x, y)->VisitGrid(cell_x, cell_y, visitor);
    }
}
#endif
