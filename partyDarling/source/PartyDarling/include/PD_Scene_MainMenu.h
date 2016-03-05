#pragma once

#include <Scene.h>
#include <shader/Shader.h>
#include <UILayer.h>
#include <Font.h>

class ComponentShaderText;
class PD_UI_ConfirmNewGame;
class PD_UI_Text;

class PD_Scene_MainMenu: public Scene {
public:		

	Shader * screenSurfaceShader;
	RenderSurface* screenSurface;
	StandardFrameBuffer* screenFBO;

	ComponentShaderText * textShader;

	UILayer * uiLayer;
	Font* menuFont;

	PD_UI_Text * joinPartyText;
	PD_UI_Text * continueText;
	PD_UI_Text * optionsText;
	PD_UI_Text * callNightText;

	NodeUI * screen;
	PD_UI_ConfirmNewGame * confirmNewGame;

	bool savedGame;

	explicit PD_Scene_MainMenu(Game * _game);
	~PD_Scene_MainMenu();

	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;

	virtual void load() override;
	virtual void unload() override;

	void showConfirmBox();
	void hideConfirmBox();
};
