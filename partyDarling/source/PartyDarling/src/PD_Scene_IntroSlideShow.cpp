#pragma once

#include <PD_Scene_IntroSlideShow.h>
#include <PD_Scene_IntermissionSlideShow.h>
#include <PD_ResourceManager.h>
#include <PD_Game.h>
#include <sweet/UI.h>
#include <sweet/Input.h>
#include <Timeout.h>

PD_Scene_IntroSlideShow::PD_Scene_IntroSlideShow(Game * _game) :
	Scene_SlideShow(_game),
	isForwads(true)
{
	// get the slides
	Texture * tex;
	tex = new Texture("assets/textures/introSlides/1.png", false, true);
	tex->load();
	push(new Slide(tex, 0));
	tex = new Texture("assets/textures/introSlides/2.png", false, true);
	tex->load();
	push(new Slide(tex, 0, PD_ResourceManager::scenario->getAudio("OPEN-ENVELOPE")->sound));
	tex = new Texture("assets/textures/introSlides/3.png", false, true);
	tex->load();
	push(new Slide(tex, 0, PD_ResourceManager::scenario->getAudio("OPEN-ENVELOPE")->sound));
	tex = new Texture("assets/textures/introSlides/4.png", false, true);
	tex->load();
	push(new Slide(tex, 0, PD_ResourceManager::scenario->getAudio("PAPER-OUT-ENVELOPE")->sound));
	tex = new Texture("assets/textures/introSlides/6.png", false, true);
	tex->load();
	push(new Slide(tex));
	tex = new Texture("assets/textures/introSlides/7.png", false, true);
	tex->load();
	push(new Slide(tex));
	for(unsigned long int i = 1; i <= 7; ++i){
		tex = new Texture("assets/textures/introSlides/tut-0"+std::to_string(i)+".png", false, true);
		tex->load();
		push(new Slide(tex));
	}

	// setup the trigger for moving on to the game after the slides are done
	eventManager->addEventListener("overflow", [_game](sweet::Event * _event){
		_game->scenes["intermission"] = new PD_Scene_IntermissionSlideshow(dynamic_cast<PD_Game*>(_game), PD_Game::progressManager->plotPosition);
		_game->switchScene("intermission", true);
	});

	textShader = new ComponentShaderText(false);

	textShader->setColor(1.f, 1.f, 1.f);
	textShader->load();

	VerticalLinearLayout * vl = new VerticalLinearLayout(uiLayer->world);
	uiLayer->addChild(vl);
	vl->setRationalWidth(1.f, uiLayer);
	vl->setAutoresizeHeight();
	vl->verticalAlignment = kMIDDLE;
	vl->horizontalAlignment = kRIGHT;

	TextLabel * skip = new TextLabel(uiLayer->world, PD_ResourceManager::scenario->getFont("options-menu-sub-font")->font, textShader);
	vl->addChild(skip);
	skip->setMouseEnabled(true);
	skip->setBackgroundColour(0.5f, 0, 0);
	skip->setRationalWidth(0.25f, vl);
	skip->setHeight(PD_ResourceManager::scenario->getFont("options-menu-sub-font")->font->getLineHeight()*2.f);
	skip->verticalAlignment = kMIDDLE;
	skip->horizontalAlignment = kCENTER;
	skip->setText("skip");
	skip->eventManager->addEventListener("click", [this](sweet::Event * _event){
		eventManager->triggerEvent("overflow");
	});
	uiLayer->invalidateLayout();

	// advance to the first slide
	changeSlide(true);

	changeSlideTimeout = new Timeout(0.25f, [this](sweet::Event * _event){
		changeSlide(isForwads);
	});
	childTransform->addChild(changeSlideTimeout, false);

	eventManager->addEventListener("changeSlide", [this](sweet::Event * _event){
		isForwads = _event->getIntData("isForwards");
		int idx = getCurrentIndex();
		if(idx > 0 && idx < 3){
			std::stringstream s;
			s << "idx: " << idx;
			Log::info(s.str());
			changeSlideTimeout->restart();
		}
	});
}

PD_Scene_IntroSlideShow::~PD_Scene_IntroSlideShow(){
	delete textShader;
}

void PD_Scene_IntroSlideShow::update(Step * _step){
	if(getCurrentIndex() == 0 || getCurrentIndex() >= 3){
		if(mouse->leftJustPressed()){
			changeSlide(true);
		}if(mouse->rightJustPressed()){
			changeSlide(false);
		}
	}
	Scene_SlideShow::update(_step);
}