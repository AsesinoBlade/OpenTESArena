#include <algorithm>
#include <cmath>

#include "EntityManager.h"
#include "EntityType.h"
#include "../Assets/MIFFile.h"
#include "../Math/Constants.h"
#include "../Math/MathUtils.h"
#include "../Math/Matrix4.h"
#include "../World/VoxelDataType.h"

#include "components/debug/Debug.h"

namespace
{
	constexpr int DEFAULT_CHUNK_X = 0;
	constexpr int DEFAULT_CHUNK_Y = 0;
}

EntityManager::EntityVisibilityData::EntityVisibilityData() :
	flatPosition(Double3::Zero), keyframe(EntityAnimationData::Keyframe(0, 0, 0))
{
	this->entity = nullptr;
	this->anglePercent = 0.0;
	this->stateType = EntityAnimationData::StateType::Idle;
}

template <typename T>
int EntityManager::EntityGroup<T>::getCount() const
{
	return static_cast<int>(this->entities.size());
}

template <typename T>
T *EntityManager::EntityGroup<T>::getEntityAtIndex(int index)
{
	DebugAssert(this->validEntities.size() == this->entities.size());
	DebugAssertIndex(this->validEntities, index);
	const bool isValid = this->validEntities[index];
	return isValid ? &this->entities[index] : nullptr;
}

template <typename T>
const T *EntityManager::EntityGroup<T>::getEntityAtIndex(int index) const
{
	DebugAssert(this->validEntities.size() == this->entities.size());
	DebugAssertIndex(this->validEntities, index);
	const bool isValid = this->validEntities[index];
	return isValid ? &this->entities[index] : nullptr;
}

template <typename T>
int EntityManager::EntityGroup<T>::getEntities(Entity **outEntities, int outSize)
{
	DebugAssert(outEntities != nullptr);
	DebugAssert(outSize >= 0);
	DebugAssert(this->validEntities.size() == this->entities.size());

	int writeIndex = 0;
	const int count = this->getCount();
	for (int i = 0; i < count; i++)
	{
		// Break if the destination buffer is full.
		if (writeIndex == outSize)
		{
			break;
		}

		DebugAssertIndex(this->validEntities, i);
		const bool isValid = this->validEntities[i];

		// Skip if entity is not valid.
		if (!isValid)
		{
			continue;
		}

		T &entity = this->entities[i];
		outEntities[writeIndex] = &entity;
		writeIndex++;
	}

	return writeIndex;
}

template <typename T>
int EntityManager::EntityGroup<T>::getEntities(const Entity **outEntities, int outSize) const
{
	DebugAssert(outEntities != nullptr);
	DebugAssert(outSize >= 0);
	DebugAssert(this->validEntities.size() == this->entities.size());

	int writeIndex = 0;
	const int count = this->getCount();
	for (int i = 0; i < count; i++)
	{
		// Break if the destination buffer is full.
		if (writeIndex == outSize)
		{
			break;
		}

		DebugAssertIndex(this->validEntities, i);
		const bool isValid = this->validEntities[i];

		// Skip if entity is not valid.
		if (!isValid)
		{
			continue;
		}

		const T &entity = this->entities[i];
		outEntities[writeIndex] = &entity;
		writeIndex++;
	}

	return writeIndex;
}

template <typename T>
std::optional<int> EntityManager::EntityGroup<T>::getEntityIndex(int id) const
{
	const auto iter = this->indices.find(id);
	if (iter != this->indices.end())
	{
		return iter->second;
	}
	else
	{
		return std::nullopt;
	}
}

template <typename T>
T *EntityManager::EntityGroup<T>::addEntity(int id)
{
	DebugAssert(id != EntityManager::NO_ID);
	DebugAssert(this->validEntities.size() == this->entities.size());

	// Entity ID must not already be in use.
	DebugAssert(!this->getEntityIndex(id).has_value());

	// Find available position in entities array.
	int index;
	if (this->freeIndices.size() > 0)
	{
		// Reuse a previously-owned entity slot.
		index = this->freeIndices.back();
		this->freeIndices.pop_back();

		DebugAssertIndex(this->validEntities, index);
		this->validEntities[index] = true;
	}
	else
	{
		// Insert new at the end of the entities list.
		index = static_cast<int>(this->entities.size());
		this->entities.push_back(T());
		this->validEntities.push_back(true);
	}

	// Initialize basic entity data.
	DebugAssertIndex(this->entities, index);
	T &entitySlot = this->entities[index];
	entitySlot.reset();
	entitySlot.setID(id);

	// Insert into ID -> entity index table.
	this->indices.insert(std::make_pair(id, index));

	return &entitySlot;
}

template <typename T>
void EntityManager::EntityGroup<T>::remove(int id)
{
	DebugAssert(id != EntityManager::NO_ID);
	DebugAssert(this->validEntities.size() == this->entities.size());

	// Find entity with the given ID.
	const std::optional<int> optIndex = this->getEntityIndex(id);
	if (optIndex.has_value())
	{
		const int index = optIndex.value();

		// Clear entity slot.
		DebugAssertIndex(this->entities, index);
		this->entities[index].reset();
		this->validEntities[index] = false;

		// Clear ID mapping.
		this->indices.erase(id);

		// Add entity index to previously-owned slots list.
		this->freeIndices.push_back(index);
	}
	else
	{
		// Not in entity group.
		DebugLogWarning("Tried to remove missing entity \"" + std::to_string(id) + "\".");
	}
}

template <typename T>
void EntityManager::EntityGroup<T>::clear()
{
	this->entities.clear();
	this->validEntities.clear();
	this->indices.clear();
	this->freeIndices.clear();
}

const int EntityManager::NO_ID = -1;

void EntityManager::init(int chunkCountX, int chunkCountY)
{
	this->staticGroups.init(chunkCountX, chunkCountY);
	this->dynamicGroups.init(chunkCountX, chunkCountY);
	this->nextID = 0;
}

int EntityManager::nextFreeID()
{
	// Check if any pre-owned entity IDs are available.
	if (this->freeIDs.size() > 0)
	{
		const int id = this->freeIDs.back();
		this->freeIDs.pop_back();
		return id;
	}
	else
	{
		// Get the next available ID.
		const int id = this->nextID;
		this->nextID++;
		return id;
	}
}

bool EntityManager::isValidChunk(int chunkX, int chunkY) const
{
	return (chunkX >= 0) && (chunkX < this->staticGroups.getWidth()) &&
		(chunkY >= 0) && (chunkY < this->staticGroups.getHeight());
}

StaticEntity *EntityManager::makeStaticEntity()
{
	const int id = this->nextFreeID();
	auto &staticGroup = this->staticGroups.get(DEFAULT_CHUNK_X, DEFAULT_CHUNK_Y);
	StaticEntity *entity = staticGroup.addEntity(id);
	DebugAssert(entity->getID() == id);
	return entity;
}

DynamicEntity *EntityManager::makeDynamicEntity()
{
	const int id = this->nextFreeID();
	auto &dynamicGroup = this->dynamicGroups.get(DEFAULT_CHUNK_X, DEFAULT_CHUNK_Y);
	DynamicEntity *entity = dynamicGroup.addEntity(id);
	DebugAssert(entity->getID() == id);
	return entity;
}

Entity *EntityManager::get(int id)
{
	DebugAssert(this->staticGroups.getWidth() == this->dynamicGroups.getWidth());
	DebugAssert(this->staticGroups.getHeight() == this->dynamicGroups.getHeight());

	// Find which entity group the given entity ID is in. This is a slow look-up because there is
	// no hint where the entity is at.
	for (int y = 0; y < this->staticGroups.getHeight(); y++)
	{
		for (int x = 0; x < this->staticGroups.getWidth(); x++)
		{
			auto &staticGroup = this->staticGroups.get(x, y);
			std::optional<int> entityIndex = staticGroup.getEntityIndex(id);
			if (entityIndex.has_value())
			{
				// Static entity.
				return staticGroup.getEntityAtIndex(*entityIndex);
			}

			auto &dynamicGroup = this->dynamicGroups.get(x, y);
			entityIndex = dynamicGroup.getEntityIndex(id);
			if (entityIndex.has_value())
			{
				// Dynamic entity.
				return dynamicGroup.getEntityAtIndex(*entityIndex);
			}
		}
	}

	// Not in any entity group.
	return nullptr;
}

const Entity *EntityManager::get(int id) const
{
	DebugAssert(this->staticGroups.getWidth() == this->dynamicGroups.getWidth());
	DebugAssert(this->staticGroups.getHeight() == this->dynamicGroups.getHeight());

	// Find which entity group the given entity ID is in. This is a slow look-up because there is
	// no hint where the entity is at.
	for (int y = 0; y < this->staticGroups.getHeight(); y++)
	{
		for (int x = 0; x < this->staticGroups.getWidth(); x++)
		{
			const auto &staticGroup = this->staticGroups.get(x, y);
			std::optional<int> entityIndex = staticGroup.getEntityIndex(id);
			if (entityIndex.has_value())
			{
				// Static entity.
				return staticGroup.getEntityAtIndex(*entityIndex);
			}

			const auto &dynamicGroup = this->dynamicGroups.get(x, y);
			entityIndex = dynamicGroup.getEntityIndex(id);
			if (entityIndex.has_value())
			{
				// Dynamic entity.
				return dynamicGroup.getEntityAtIndex(*entityIndex);
			}
		}
	}

	// Not in any entity group.
	return nullptr;
}

int EntityManager::getCount(EntityType entityType) const
{
	auto getCountFromGroups = [](const auto &entityGroups)
	{
		int count = 0;
		for (int y = 0; y < entityGroups.getHeight(); y++)
		{
			for (int x = 0; x < entityGroups.getWidth(); x++)
			{
				const auto &entityGroup = entityGroups.get(x, y);
				count += entityGroup.getCount();
			}
		}

		return count;
	};

	switch (entityType)
	{
	case EntityType::Static:
		return getCountFromGroups(this->staticGroups);
	case EntityType::Dynamic:
		return getCountFromGroups(this->dynamicGroups);
	default:
		DebugUnhandledReturnMsg(int, std::to_string(static_cast<int>(entityType)));
	}
}

int EntityManager::getTotalCountInChunk(int chunkX, int chunkY) const
{
	DebugAssert(this->staticGroups.getWidth() == this->dynamicGroups.getWidth());
	DebugAssert(this->staticGroups.getHeight() == this->dynamicGroups.getHeight());

	if (!this->isValidChunk(chunkX, chunkY))
	{
		return 0;
	}

	const auto &staticGroup = this->staticGroups.get(chunkX, chunkY);
	const auto &dynamicGroup = this->dynamicGroups.get(chunkX, chunkY);
	return staticGroup.getCount() + dynamicGroup.getCount();
}

int EntityManager::getTotalCount() const
{
	return this->getCount(EntityType::Static) + this->getCount(EntityType::Dynamic);
}

int EntityManager::getEntities(EntityType entityType, Entity **outEntities, int outSize)
{
	DebugAssert(outEntities != nullptr);
	DebugAssert(outSize >= 0);

	auto getEntitiesFromGroups = [](auto &entityGroups, Entity **outEntities, int outSize)
	{
		int writeIndex = 0;
		for (int y = 0; y < entityGroups.getHeight(); y++)
		{
			for (int x = 0; x < entityGroups.getWidth(); x++)
			{
				if (writeIndex == outSize)
				{
					break;
				}

				auto &entityGroup = entityGroups.get(x, y);
				writeIndex += entityGroup.getEntities(outEntities + writeIndex, outSize - writeIndex);
			}
		}

		return writeIndex;
	};

	// Get entities from the desired type.
	switch (entityType)
	{
	case EntityType::Static:
		return getEntitiesFromGroups(this->staticGroups, outEntities, outSize);
	case EntityType::Dynamic:
		return getEntitiesFromGroups(this->dynamicGroups, outEntities, outSize);
	default:
		DebugUnhandledReturnMsg(int, std::to_string(static_cast<int>(entityType)));
	}
}

int EntityManager::getEntities(EntityType entityType, const Entity **outEntities, int outSize) const
{
	DebugAssert(outEntities != nullptr);
	DebugAssert(outSize >= 0);

	auto getEntitiesFromGroups = [](const auto &entityGroups, const Entity **outEntities, int outSize)
	{
		int writeIndex = 0;
		for (int y = 0; y < entityGroups.getHeight(); y++)
		{
			for (int x = 0; x < entityGroups.getWidth(); x++)
			{
				if (writeIndex == outSize)
				{
					break;
				}

				const auto &entityGroup = entityGroups.get(x, y);
				writeIndex += entityGroup.getEntities(outEntities + writeIndex, outSize - writeIndex);
			}
		}

		return writeIndex;
	};

	// Get entities from the desired type.
	switch (entityType)
	{
	case EntityType::Static:
		return getEntitiesFromGroups(this->staticGroups, outEntities, outSize);
	case EntityType::Dynamic:
		return getEntitiesFromGroups(this->dynamicGroups, outEntities, outSize);
	default:
		DebugUnhandledReturnMsg(int, std::to_string(static_cast<int>(entityType)));
	}
}

int EntityManager::getTotalEntitiesInChunk(int chunkX, int chunkY, const Entity **outEntities, int outSize) const
{
	if (!this->isValidChunk(chunkX, chunkY))
	{
		return 0;
	}

	// Can't assume the given pointer is not null.
	if ((outEntities == nullptr) || (outSize == 0))
	{
		return 0;
	}

	// Fill the output buffer with as many entities as will fit.
	int writeIndex = 0;
	auto tryWriteEntities = [outEntities, outSize, &writeIndex](const auto &entityGroup)
	{
		const int entityCount = entityGroup.getCount();

		for (int i = 0; i < entityCount; i++)
		{
			// Break if the output buffer is full.
			if (writeIndex == outSize)
			{
				break;
			}

			outEntities[writeIndex] = entityGroup.getEntityAtIndex(i);
			writeIndex++;
		}
	};

	auto &staticGroup = this->staticGroups.get(chunkX, chunkY);
	auto &dynamicGroup = this->dynamicGroups.get(chunkX, chunkY);
	tryWriteEntities(staticGroup);
	tryWriteEntities(dynamicGroup);

	return writeIndex;
}

int EntityManager::getTotalEntities(const Entity **outEntities, int outSize) const
{
	// Apparently std::vector::data() can be either null or non-null when the container is empty,
	// so can't assume the given pointer is not null.
	if ((outEntities == nullptr) || (outSize == 0))
	{
		return 0;
	}

	// Fill the output buffer with as many entities as will fit.
	int writeIndex = 0;
	for (int y = 0; y < this->staticGroups.getHeight(); y++)
	{
		for (int x = 0; x < this->staticGroups.getWidth(); x++)
		{
			if (writeIndex == outSize)
			{
				break;
			}

			writeIndex += this->getTotalEntitiesInChunk(x, y, outEntities + writeIndex, outSize - writeIndex);
		}
	}

	return writeIndex;
}

const EntityDefinition *EntityManager::getEntityDef(int flatIndex) const
{
	const auto iter = std::find_if(this->entityDefs.begin(), this->entityDefs.end(),
		[flatIndex](const EntityDefinition &def)
	{
		return def.getInfData().flatIndex == flatIndex;
	});

	return (iter != this->entityDefs.end()) ? &(*iter) : nullptr;
}

EntityDefinition *EntityManager::addEntityDef(EntityDefinition &&def)
{
	this->entityDefs.push_back(std::move(def));
	return &this->entityDefs.back();
}

void EntityManager::getEntityVisibilityData(const Entity &entity, const Double2 &eye2D,
	double ceilingHeight, const VoxelGrid &voxelGrid, EntityVisibilityData &outVisData) const
{
	outVisData.entity = &entity;
	const EntityDefinition &entityDef = *this->getEntityDef(entity.getDataIndex());
	const EntityAnimationData &entityAnimData = entityDef.getAnimationData();

	// Get active state
	const EntityAnimationData::Instance &animInstance = entity.getAnimation();
	const std::vector<EntityAnimationData::State> &stateList = animInstance.getStateList(entityAnimData);
	const int stateCount = static_cast<int>(stateList.size()); // 1 if it's the same for all angles.

	// Calculate state index based on entity direction relative to camera.
	const double animAngle = [&entity, &eye2D, stateCount]()
	{
		if (entity.getEntityType() == EntityType::Static)
		{
			// Static entities always face the camera.
			return 0.0;
		}
		else if (entity.getEntityType() == EntityType::Dynamic)
		{
			// Dynamic entities are angle-dependent.
			const DynamicEntity &dynamicEntity = static_cast<const DynamicEntity&>(entity);
			const Double2 &entityDir = dynamicEntity.getDirection();
			const Double2 diffDir = (eye2D - entity.getPosition()).normalized();

			const double entityAngle = MathUtils::fullAtan2(entityDir.y, entityDir.x);
			const double diffAngle = MathUtils::fullAtan2(diffDir.y, diffDir.x);

			// Use the difference of the two vectors as the angle vector.
			const Double2 resultDir = entityDir - diffDir;
			const double resultAngle = Constants::Pi + MathUtils::fullAtan2(resultDir.y, resultDir.x);

			// Angle bias so the final direction is centered within its angle range.
			const double angleBias = (Constants::TwoPi / static_cast<double>(stateCount)) * 0.50;

			return std::fmod(resultAngle + angleBias, Constants::TwoPi);
		}
		else
		{
			DebugUnhandledReturnMsg(double,
				std::to_string(static_cast<int>(entity.getEntityType())));
		}
	}();

	outVisData.anglePercent = std::clamp(animAngle / Constants::TwoPi, 0.0, Constants::JustBelowOne);

	const int stateIndex = [&outVisData, stateCount]()
	{
		const int index = static_cast<int>(static_cast<double>(stateCount) * outVisData.anglePercent);
		return std::clamp(index, 0, stateCount - 1);
	}();

	DebugAssertIndex(stateList, stateIndex);
	const EntityAnimationData::State &animState = stateList[stateIndex];
	outVisData.stateType = animState.getType();

	// Get the entity's current animation frame (dimensions, texture, etc.).
	outVisData.keyframe = [&entity, &entityAnimData, &animInstance, stateIndex, &animState]()
		-> const EntityAnimationData::Keyframe&
	{
		const int keyframeIndex = animInstance.getKeyframeIndex(stateIndex, entityAnimData);
		const BufferView<const EntityAnimationData::Keyframe> keyframes = animState.getKeyframes();
		return keyframes.get(keyframeIndex);
	}();

	const double flatWidth = outVisData.keyframe.getWidth();
	const double flatHeight = outVisData.keyframe.getHeight();
	const double flatHalfWidth = flatWidth * 0.50;

	const Double2 &entityPos = entity.getPosition();
	const double entityPosX = entityPos.x;
	const double entityPosZ = entityPos.y;

	const double flatYOffset = static_cast<double>(-entityDef.getInfData().yOffset) / MIFFile::ARENA_UNITS;

	// If the entity is in a raised platform voxel, they are set on top of it.
	const double raisedPlatformYOffset = [ceilingHeight, &voxelGrid, &entityPos]()
	{
		const Int2 entityVoxelPos(
			static_cast<int>(entityPos.x),
			static_cast<int>(entityPos.y));
		const uint16_t voxelID = voxelGrid.getVoxel(entityVoxelPos.x, 1, entityVoxelPos.y);
		const VoxelDefinition &voxelDef = voxelGrid.getVoxelDef(voxelID);

		if (voxelDef.dataType == VoxelDataType::Raised)
		{
			const VoxelDefinition::RaisedData &raised = voxelDef.raised;
			return (raised.yOffset + raised.ySize) * ceilingHeight;
		}
		else
		{
			// No raised platform offset.
			return 0.0;
		}
	}();

	// Bottom center of flat.
	outVisData.flatPosition = Double3(
		entityPosX,
		ceilingHeight + flatYOffset + raisedPlatformYOffset,
		entityPosZ);
}

void EntityManager::getEntityBoundingBox(const Entity &entity, const EntityVisibilityData &visData,
	Double3 *outMin, Double3 *outMax) const
{
	// Start with a bounding cylinder.
	const double radius = visData.keyframe.getWidth() / 2.0;
	const double height = visData.keyframe.getHeight();

	// Convert the bounding cylinder to an axis-aligned bounding box.
	outMin->x = visData.flatPosition.x - radius;
	outMin->y = visData.flatPosition.y;
	outMin->z = visData.flatPosition.z - radius;
	outMax->x = visData.flatPosition.x + radius;
	outMax->y = visData.flatPosition.y + height;
	outMax->z = visData.flatPosition.z + radius;
}

void EntityManager::remove(int id)
{
	DebugAssert(this->staticGroups.getWidth() == this->dynamicGroups.getWidth());
	DebugAssert(this->staticGroups.getHeight() == this->dynamicGroups.getHeight());

	// Find which entity group the given entity ID is in. This is a slow look-up because there is
	// no hint where the entity is at.
	for (int y = 0; y < this->staticGroups.getHeight(); y++)
	{
		for (int x = 0; x < this->staticGroups.getWidth(); x++)
		{
			auto &staticGroup = this->staticGroups.get(x, y);
			std::optional<int> entityIndex = staticGroup.getEntityIndex(id);
			if (entityIndex.has_value())
			{
				// Static entity.
				staticGroup.remove(id);

				// Insert entity ID into the free list.
				this->freeIDs.push_back(id);
				return;
			}

			auto &dynamicGroup = this->dynamicGroups.get(x, y);
			entityIndex = dynamicGroup.getEntityIndex(id);
			if (entityIndex.has_value())
			{
				// Dynamic entity.
				dynamicGroup.remove(id);

				// Insert entity ID into the free list.
				this->freeIDs.push_back(id);
				return;
			}
		}
	}

	// Not in any entity group.
	DebugLogWarning("Tried to remove missing entity \"" + std::to_string(id) + "\".");
}

void EntityManager::clear()
{
	for (int y = 0; y < this->staticGroups.getHeight(); y++)
	{
		for (int x = 0; x < this->staticGroups.getWidth(); x++)
		{
			auto &staticGroup = this->staticGroups.get(x, y);
			auto &dynamicGroup = this->dynamicGroups.get(x, y);
			staticGroup.clear();
			dynamicGroup.clear();
		}
	}

	this->entityDefs.clear();

	this->freeIDs.clear();
	this->nextID = 0;
}

void EntityManager::tick(Game &game, double dt)
{
	// @todo: probably only want to tick entities near the player, so get the chunks near the player.
	auto tickEntityGroups = [&game, dt](auto &entityGroups)
	{
		for (int y = 0; y < entityGroups.getHeight(); y++)
		{
			for (int x = 0; x < entityGroups.getWidth(); x++)
			{
				auto &entityGroup = entityGroups.get(x, y);
				const int entityCount = entityGroup.getCount();

				for (int i = 0; i < entityCount; i++)
				{
					auto *entity = entityGroup.getEntityAtIndex(i);
					if (entity != nullptr)
					{
						entity->tick(game, dt);
					}
				}
			}
		}
	};

	tickEntityGroups(this->staticGroups);
	tickEntityGroups(this->dynamicGroups);
}
