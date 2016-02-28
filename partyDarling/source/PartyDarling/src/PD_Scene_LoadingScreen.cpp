#pragma once

#include "PD_Scene_LoadingScreen.h"
#include <StandardFrameBuffer.h>
#include <RenderSurface.h>
#include <RenderOptions.h>
#include <Game.h>
#include <PD_ResourceManager.h>
#include <VerticalLinearLayout.h>
#include <shader/ShaderComponentText.h>
#include <shader/ComponentShaderText.h>
#include <TextArea.h>
#include <Game.h>
#include <PD_UI_Text.h>
#include <PD_Scene_Main.h>

#include <MeshFactory.h>

PD_PhraseGenerator_Loading::PD_PhraseGenerator_Loading(){
	makeDatabases("assets/wordlists/loading.json");
}

std::string PD_PhraseGenerator_Loading::getMessage(unsigned long int _phase) {
	return replaceWords(escapeChar + std::to_string(_phase) + escapeChar);
}

class PD_UI_Text;

PD_Scene_LoadingScreen::PD_Scene_LoadingScreen(Game * _game) :
	Scene(_game),
	screenSurfaceShader(new Shader("assets/engine basics/DefaultRenderSurface", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader)),
	screenFBO(new StandardFrameBuffer(true)), 
	textShader(new ComponentShaderText(true)),
	uiLayer(0,0,0,0),
	menuFont(PD_ResourceManager::scenario->getFont("main-menu-font")->font),
	loadingPercent(0),
	lastMessagePhase(-1),
	lastMessageTime(-TIME_PER_MESSAGE)
{
	screenSurfaceShader->referenceCount++;
	screenFBO->referenceCount++;
	screenSurface->referenceCount++;


	glm::uvec2 sd = sweet::getWindowDimensions();
	uiLayer.resize(0,sd.x,0,sd.y);

	VerticalLinearLayout * layout = new VerticalLinearLayout(uiLayer.world);
	uiLayer.addChild(layout);
	layout->horizontalAlignment = kCENTER;
	layout->verticalAlignment = kMIDDLE;
	layout->setRationalWidth(1.f, &uiLayer);
	layout->setRationalHeight(1.f, &uiLayer);

	textShader->setColor(1,1,1);
	loadingMessage = new TextLabel(uiLayer.world, menuFont, textShader);
	layout->addChild(loadingMessage);
	loadingMessage->setMarginBottom(15);
	
	// slider
	loadingSlider = new SliderControlled(uiLayer.world, &loadingPercent, 0, 1);
	layout->addChild(loadingSlider);
	loadingSlider->setRationalWidth(0.5f, layout);
	loadingSlider->setPixelHeight(25);

	loadingSlider->thumb->background->meshTransform->scale(2);
	loadingSlider->thumb->setBackgroundColour(1,1,1,1);
	loadingSlider->thumb->background->mesh->pushTexture2D(PD_ResourceManager::scenario->getTexture("SLIDER-THUMB")->texture);
	
	loadingSlider->setBackgroundColour(1,1,1,1);
	loadingSlider->background->mesh->pushTexture2D(PD_ResourceManager::scenario->getTexture("SLIDER-TRACK")->texture);
	
	loadingSlider->fill->setBackgroundColour(1,1,1,1);
	loadingSlider->fill->background->mesh->pushTexture2D(PD_ResourceManager::scenario->getTexture("SLIDER-FILL")->texture);

	// bg
	layout->background->setVisible(true);
	layout->setBackgroundColour(1,1,1,1);
	layout->background->mesh->pushTexture2D(PD_ResourceManager::scenario->getTexture("loading-background")->texture);
	layout->background->mesh->setScaleMode(GL_NEAREST);

	uiLayer.invalidateLayout();
}

PD_Scene_LoadingScreen::~PD_Scene_LoadingScreen() {
	deleteChildTransform();
	screenSurfaceShader->decrementAndDelete();
	screenFBO->decrementAndDelete();
	screenSurface->decrementAndDelete();
}

void PD_Scene_LoadingScreen::update(Step* _step) {

	Scene::update(_step);

	glm::uvec2 sd = sweet::getWindowDimensions();
	uiLayer.resize(0,sd.x,0,sd.y);
	uiLayer.update(_step);
}

void PD_Scene_LoadingScreen::render(sweet::MatrixStack* _matrixStack, RenderOptions* _renderOptions) {
	screenFBO->resize(game->viewPortWidth, game->viewPortHeight);
	

	FrameBufferInterface::pushFbo(screenFBO);
	
	_renderOptions->setClearColour(1,0,1,1);
	_renderOptions->clear();

	Scene::render(_matrixStack, _renderOptions);
	uiLayer.render(_matrixStack, _renderOptions);
	
	FrameBufferInterface::popFbo();


	screenSurface->render(screenFBO->getTextureId());
}

void PD_Scene_LoadingScreen::load() {
	Scene::load();	
	uiLayer.load();
	screenSurface->load();
	screenSurfaceShader->load();
	screenFBO->load();
}

void PD_Scene_LoadingScreen::unload() {
	uiLayer.unload();
	screenSurface->unload();
	screenSurfaceShader->unload();
	screenFBO->unload();
	Scene::unload();	
}

void PD_Scene_LoadingScreen::updateProgress(float _progress){
	double t = glfwGetTime();
	unsigned long int phase = glm::clamp((int)glm::round(loadingPercent*LOADING_PHASES), 1, LOADING_PHASES);
	loadingPercent = _progress;
	if(t - lastMessageTime >= TIME_PER_MESSAGE){
		loadingMessage->setText(loadingMessages.getMessage(phase));
		lastMessagePhase = phase;
		lastMessageTime = t;
	}
	uiLayer.invalidateLayout();
}