#pragma once

#include <PD_UI_VolumeControl.h>
#include <PD_ResourceManager.h>
#include <PD_Game.h>
#include <OpenAlSound.h>

PD_UI_VolumeControl::PD_UI_VolumeControl(BulletWorld * _world, Shader * _textShader) :
	VerticalLinearLayout(_world),
	textShader(_textShader)
{
	verticalAlignment = kMIDDLE;

	addSlider("master", &OpenAL_Sound::masterGain);
	for(auto & s : OpenAL_Sound::categoricalGain){
		addSlider(s.first, &s.second);
	}
	invalidateLayout();
}

void PD_UI_VolumeControl::addSlider(std::string _text, float * _target){
	VerticalLinearLayout * container = new VerticalLinearLayout(world);
	addChild(container);
	container->verticalAlignment = kMIDDLE;
	container->horizontalAlignment = kLEFT;
	container->setRationalWidth(1.f, this);
	container->setRationalHeight(1.f / ((OpenAL_Sound::categoricalGain.size() + 1)), this);

	TextLabel * label = new TextLabel(world, PD_ResourceManager::scenario->getFont("options-menu-sub-font")->font, textShader);
	SliderController * slider = new SliderController(world, _target, *_target, 0.f, 2.f);

	container->addChild(label);
	container->addChild(slider);

	label->setText(_text);
	label->setRationalWidth(1.f, container);
	label->setRationalHeight(0.6f, container);
	slider->setRationalWidth(1.f, container);
	slider->setRationalHeight(0.4f, container);

	slider->thumb->background->meshTransform->scale(4)->translate(glm::vec3(-1,0,0));
	slider->thumb->setBackgroundColour(1,1,1,1);
	slider->thumb->background->mesh->pushTexture2D(PD_ResourceManager::scenario->getTexture("SLIDER-THUMB")->texture);
	slider->thumb->background->mesh->setScaleMode(GL_NEAREST);

	slider->setBackgroundColour(1,1,1,1);
	slider->background->mesh->pushTexture2D(PD_ResourceManager::scenario->getTexture("SLIDER-TRACK")->texture);
	slider->background->mesh->setScaleMode(GL_NEAREST);

	slider->fill->setBackgroundColour(1,1,1,1);
	slider->fill->background->mesh->pushTexture2D(PD_ResourceManager::scenario->getTexture("SLIDER-FILL")->texture);
	slider->fill->background->mesh->setScaleMode(GL_NEAREST);

	// make sure the background music updates when the music or master slider is changed
	if(_text == "music" || _text == "master"){
		slider->eventManager->addEventListener("change", [](sweet::Event * _event){
			if(PD_Game::bgmTrack != nullptr){
				PD_Game::bgmTrack->pause();
				PD_Game::bgmTrack->play();
			}
		});
	}
}