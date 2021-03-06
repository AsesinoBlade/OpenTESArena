#ifndef MAIN_QUEST_SPLASH_PANEL_H
#define MAIN_QUEST_SPLASH_PANEL_H

#include <string>

#include "Panel.h"
#include "../Assets/TextureAssetReference.h"
#include "../UI/Button.h"
#include "../UI/TextBox.h"

class MainQuestSplashPanel : public Panel
{
private:
	TextBox textBox;
	Button<Game&> exitButton;
	TextureAssetReference splashTextureAssetRef;
public:
	MainQuestSplashPanel(Game &game);
	~MainQuestSplashPanel() override = default;

	bool init(int provinceID);

	virtual std::optional<CursorData> getCurrentCursor() const override;
	virtual void handleEvent(const SDL_Event &e) override;
	virtual void render(Renderer &renderer) override;
};

#endif
