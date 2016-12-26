#ifndef TEXTURE_NAME_H
#define TEXTURE_NAME_H

// A unique identifier for every non-bulk image file; non-bulk meaning not wall 
// or sprite textures. There are hundreds of bulk textures and using an enum for 
// each would be prohibitive.

// Bulk textures should use some kind of filename indexing method that is loaded
// from a text file, associating each wall or sprite with a set of filenames.

// We don't want to have texture names for single-image sprites. Just include
// those with the bulk textures, too.

enum class TextureName
{
	// Character heads for the character sheet.
	FemaleArgonianHeads,
	FemaleBretonHeads,
	FemaleDarkElfHeads,
	FemaleHighElfHeads,
	FemaleKhajiitHeads,
	FemaleNordHeads,
	FemaleRedguardHeads,
	FemaleWoodElfHeads,
	MaleArgonianHeads,
	MaleBretonHeads,
	MaleDarkElfHeads,
	MaleHighElfHeads,
	MaleKhajiitHeads,
	MaleNordHeads,
	MaleRedguardHeads,
	MaleWoodElfHeads,

	// Character heads for the in-game interface.
	FemaleArgonianTrimmedHeads,
	FemaleBretonTrimmedHeads,
	FemaleDarkElfTrimmedHeads,
	FemaleHighElfTrimmedHeads,
	FemaleKhajiitTrimmedHeads,
	FemaleNordTrimmedHeads,
	FemaleRedguardTrimmedHeads,
	FemaleWoodElfTrimmedHeads,
	MaleArgonianTrimmedHeads,
	MaleBretonTrimmedHeads,
	MaleDarkElfTrimmedHeads,
	MaleHighElfTrimmedHeads,
	MaleKhajiitTrimmedHeads,
	MaleNordTrimmedHeads,
	MaleRedguardTrimmedHeads,
	MaleWoodElfTrimmedHeads,

	// Character backgrounds (without clothes).
	FemaleArgonianBackground,
	FemaleBretonBackground,
	FemaleDarkElfBackground,
	FemaleHighElfBackground,
	FemaleKhajiitBackground,
	FemaleNordBackground,
	FemaleRedguardBackground,
	FemaleWoodElfBackground,
	MaleArgonianBackground,
	MaleBretonBackground,
	MaleDarkElfBackground,
	MaleHighElfBackground,
	MaleKhajiitBackground,
	MaleNordBackground,
	MaleRedguardBackground,
	MaleWoodElfBackground,

	// Cursors.
	ArrowCursors,
	QuillCursor,
	SwordCursor,

	// Equipment overlays.
	FemaleEquipment,
	FemaleHelmet, // Unused in original.
	FemaleNonMagicShirt,
	FemaleMagicShirt,
	FemaleDemoTop, // Used in Arena demo image.
	FemalePants,
	MaleEquipment,
	MaleHelmet, // Unused in original.
	MaleNonMagicShirt,
	MaleMagicShirt,
	MalePants,

	// Interface.
	AcceptReject, // Accept/Reject for bartering.
	AcceptCounterReject, // Accept/Counter/Reject for bartering.
	AddJobStatusCancel, // Blacksmith repair buttons.
	BarterBackground, // Stone-looking background for bartering.
	BonusPointsText, // Texture shown in character stats on level-up.
	CharacterCreation, // Starry night with house.
	CharacterEquipment, // Character sheet equipment. "NEWEQUIP.IMG" switches Drop and Exit.
	CharacterStats, // Character sheet attributes, stats, etc..
	CompassFrame, // Compass border with "gargoyle-like" object.
	CompassSlider, // Actual compass headings.
	GameWorldInterface, // Portrait, stat bars, buttons in game world.
	IntroTitle, // Arena copyright, etc..
	IntroQuote, // "The best techniques...".
	LoadSave, // Slots for loading and saving.
	Logbook, // Logbook background.
	MainMenu, // Load, New, Exit.
	NextPage, // Next page button in character stats.
	NoExit, // Covers "Exit" in race select.
	NoSpell, // Darkened spell icon for non-spellcasters.
	ParchmentBig, // Fullscreen parchment with scrolls on top and bottom.
	ParchmentCorner, // Helps with parchment generation.
	PauseBackground, // Arena logo with sound/music/detail and buttons
	PopUp,
	PopUp2, // For character creation classes.
	PopUp11, // For items, etc..
	RaceSelect, // World map with location dots and no exit text.
	SpellbookText, // Text that covers up "Equipment" in player's inventory.
	StatusGradients, // Colored gradients behind player portrait.
	UpDown, // Scroll arrows.
	YesNoCancel, // Yes/No/Cancel texture for bartering.

	// Main quest dungeon splash screens.
	CryptOfHeartsSplash,
	CrystalTowerSplash,
	DagothUrSplash,
	EldenGroveSplash,
	FangLairSplash,
	HallsOfColossusSplash,
	LabyrinthianSplash,
	MurkwoodSplash,

	// Maps.
	Automap,
	BlackMarshMap,
	ElsweyrMap,
	HammerfellMap,
	HighRockMap,
	ImperialProvinceMap,
	MorrowindMap,
	SkyrimMap,
	SummersetIsleMap,
	ValenwoodMap,
	WorldMap, // World map without location dots and with exit text.

	// Map icons.
	CityStateIcon,
	DungeonIcon,
	TownIcon,
	VillageIcon,

	// Spellbook and spellmaker.
	BuySpellBackground,
	Form1,
	Form2,
	Form3,
	Form4,
	Form4A,
	Form5,
	Form6,
	Form6A,
	Form7,
	Form8,
	Form9,
	Form10,
	Form11,
	Form12,
	Form13,
	Form14,
	Form15,
	SpellMakerBackground,

	// Weapon animations.
	ArrowsAnimation, // Idle bow animation (ARROWFUL.CFA is unused).
	AxeAnimation,
	ChainAnimation, // Fists with chainmail (unused).
	FistsAnimation,
	FlailAnimation,
	HammerAnimation,
	MaceAnimation,
	PlateAnimation, // Fists with plate mail (unused).
	StaffAnimation,
	SwordAnimation,
};

#endif
