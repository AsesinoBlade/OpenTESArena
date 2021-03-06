#include "AutomapPanel.h"
#include "CharacterPanel.h"
#include "GameWorldPanel.h"
#include "GameWorldUiController.h"
#include "GameWorldUiModel.h"
#include "GameWorldUiView.h"
#include "LogbookPanel.h"
#include "PauseMenuPanel.h"
#include "TextSubPanel.h"
#include "WorldMapPanel.h"
#include "../Entities/Player.h"
#include "../Game/ArenaClockUtils.h"
#include "../Game/ArenaDateUtils.h"
#include "../Game/Game.h"
#include "../UI/TextAlignment.h"
#include "../UI/TextBox.h"

#include "components/debug/Debug.h"
#include "components/utilities/String.h"

void GameWorldUiController::onCharacterSheetButtonSelected(Game &game)
{
	game.setPanel<CharacterPanel>();
}

void GameWorldUiController::onWeaponButtonSelected(Player &player)
{
	WeaponAnimation &weaponAnimation = player.getWeaponAnimation();

	if (weaponAnimation.isSheathed())
	{
		// Begin unsheathing the weapon.
		weaponAnimation.setState(WeaponAnimation::State::Unsheathing);
	}
	else if (weaponAnimation.isIdle())
	{
		// Begin sheathing the weapon.
		weaponAnimation.setState(WeaponAnimation::State::Sheathing);
	}
}

void GameWorldUiController::onStealButtonSelected()
{
	DebugLog("Steal.");
}

void GameWorldUiController::onStatusButtonSelected(Game &game)
{
	const std::string text = GameWorldUiModel::getStatusButtonText(game);
	const Int2 center = GameWorldUiView::getStatusPopUpTextCenterPoint(game);
	const TextBox::InitInfo textBoxInitInfo = TextBox::InitInfo::makeWithCenter(
		text,
		center,
		GameWorldUiView::StatusPopUpFontName,
		GameWorldUiView::StatusPopUpTextColor,
		GameWorldUiView::StatusPopUpTextAlignment,
		std::nullopt,
		GameWorldUiView::StatusPopUpTextLineSpacing,
		game.getFontLibrary());

	Texture texture = TextureUtils::generate(
		GameWorldUiView::StatusPopUpTexturePatternType,
		GameWorldUiView::getStatusPopUpTextureWidth(textBoxInitInfo.rect.getWidth()),
		GameWorldUiView::getStatusPopUpTextureHeight(textBoxInitInfo.rect.getHeight()),
		game.getTextureManager(),
		game.getRenderer());

	const Int2 textureCenter = center;
	game.pushSubPanel<TextSubPanel>(textBoxInitInfo, text, GameWorldUiController::onStatusPopUpSelected,
		std::move(texture), textureCenter);
}

void GameWorldUiController::onStatusPopUpSelected(Game &game)
{
	game.popSubPanel();
}

void GameWorldUiController::onMagicButtonSelected()
{
	DebugLog("Magic.");
}

void GameWorldUiController::onLogbookButtonSelected(Game &game)
{
	game.setPanel<LogbookPanel>();
}

void GameWorldUiController::onUseItemButtonSelected()
{
	DebugLog("Use item.");
}

void GameWorldUiController::onCampButtonSelected()
{
	DebugLog("Camp.");
}

void GameWorldUiController::onScrollUpButtonSelected(GameWorldPanel &panel)
{
	// Nothing yet.
}

void GameWorldUiController::onScrollDownButtonSelected(GameWorldPanel &panel)
{
	// Nothing yet.
}

void GameWorldUiController::onPauseButtonSelected(Game &game)
{
	game.setPanel<PauseMenuPanel>();
}

void GameWorldUiController::onMapButtonSelected(Game &game, bool goToAutomap)
{
	if (goToAutomap)
	{
		const auto &exeData = game.getBinaryAssetLibrary().getExeData();
		auto &gameState = game.getGameState();
		const LocationDefinition &locationDef = gameState.getLocationDefinition();
		const LocationInstance &locationInst = gameState.getLocationInstance();
		const auto &player = gameState.getPlayer();
		const MapDefinition &mapDef = gameState.getActiveMapDef();
		const MapInstance &mapInst = gameState.getActiveMapInst();
		const int activeLevelIndex = mapInst.getActiveLevelIndex();
		const LevelDefinition &levelDef = mapDef.getLevel(activeLevelIndex);
		const LevelInfoDefinition &levelInfoDef = mapDef.getLevelInfoForLevel(activeLevelIndex);
		const LevelInstance &levelInst = mapInst.getLevel(activeLevelIndex);

		// Some places (like named/wild dungeons) do not display a name on the automap.
		const std::string automapLocationName = [&gameState, &exeData, &locationDef, &locationInst]()
		{
			const std::string &locationName = locationInst.getName(locationDef);
			const bool isCity = locationDef.getType() == LocationDefinition::Type::City;
			const bool isMainQuestDungeon = locationDef.getType() == LocationDefinition::Type::MainQuestDungeon;
			return (isCity || isMainQuestDungeon) ? locationName : std::string();
		}();

		game.setPanel<AutomapPanel>(player.getPosition(), player.getGroundDirection(),
			levelInst.getChunkManager(), automapLocationName);
	}
	else
	{
		game.setPanel<WorldMapPanel>();
	}
}
