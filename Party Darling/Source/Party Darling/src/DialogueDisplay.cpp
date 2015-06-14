#pragma once

#include <DialogueDisplay.h>
#include <shader\ComponentShaderText.h>
#include <shader\ShaderComponentTexture.h>
#include <MeshInterface.h>
#include <Font.h>
#include <PD_ResourceManager.h>

DialogueDisplay::DialogueDisplay(BulletWorld * _world, Scene * _scene, Font * _font, float _width, float _height) :
	currentDialogue(0),
	waitingForInput(false)
{
	hlayout = new LinearLayout(Orientation::HORIZONTAL, _world, _scene);
	vlayout = new LinearLayout(Orientation::VERTICAL, _world, _scene);
	optionslayout = new LinearLayout(Orientation::VERTICAL, _world, _scene);

	ComponentShaderBase * backgroundShader = new ComponentShaderBase(true);
	backgroundShader->addComponent(new ShaderComponentTexture(backgroundShader));
	backgroundShader->compileShader();
	ComponentShaderText * textShader = new ComponentShaderText(true);
	textShader->textComponent->setColor(glm::vec3(1, 0, 0));
	
	portraitPanel = new UIPanel(_height*0.5f, _height*0.5f, _world, _scene);
	portraitPanel->setShader(backgroundShader, true);
	
	dialogue = new LabelV2(_world, _scene, _font, textShader, backgroundShader, _width-_height);
	speaker = new LabelV2(_world, _scene, _font, textShader, backgroundShader, _width-_height);

	hlayout->addChild(portraitPanel);
	vlayout->addChild(speaker);
	vlayout->addChild(dialogue);
	hlayout->addChild(vlayout);
	vlayout->addChild(optionslayout);
	
	childTransform->addChild(hlayout, false);

	timeout = new Timeout(2.f);
	timeout->onCompleteFunction = [this](Timeout * _this) {
		this->sayNext();
	};
}

DialogueDisplay::~DialogueDisplay(){

}

bool DialogueDisplay::sayNext(){
	// check if there's anything left to say at all
	if(currentDialogue >= stuffToSay.size()){
		return false;
	}

	// clear out the options list (they'll still be there if the last thing was an ask)
	/*while(options.size() > 0){
		delete options.back()->parents.at(0);
		options.pop_back();
	}*/

	// set the speaker and portrait
	std::string sp = stuffToSay.at(currentDialogue)->speaker;
	speaker->setText(std::wstring(sp.begin(), sp.end()));
	portraitPanel->mesh->textures.clear();
	if(stuffToSay.at(currentDialogue)->portrait == "cheryl"){
		portraitPanel->mesh->pushTexture2D(PD_ResourceManager::cheryl);
	}else{

	}

	// move to the next text in the current dialogue object
	++stuffToSay.at(currentDialogue)->currentText;
	// check if there's any text left to say in the current dialogue object
	if(stuffToSay.at(currentDialogue)->currentText < stuffToSay.at(currentDialogue)->text.size()){
		// if there is, read the text in the current dialogue and return
		std::string thingToSay = stuffToSay.at(currentDialogue)->getCurrentText();
		dialogue->setText(std::wstring(thingToSay.begin(), thingToSay.end()));

		DialogueAsk * ask = dynamic_cast<DialogueAsk *>(stuffToSay.at(currentDialogue));
		if(ask != nullptr){
			waitingForInput = true;
			for(std::string s : ask->options){
				//dialogue->appendText(std::wstring(s.begin(), s.end()));
				PD_Button * o = new PD_Button(hlayout->world, hlayout->scene);
				o->normalLabel->setText(std::wstring(s.begin(), s.end()));
				options.push_back(o);
				optionslayout->addChild(o);
				//o->parents.at(0)->scale(50,50,1);
				o->onDownFunction = [this](NodeUI * _this) {
					this->waitingForInput = false;
					optionslayout->childTransform->children.clear();
					sayNext();
				};
			}
		}

		if(!waitingForInput){
			timeout->restart();
		}

		return true;
	}else{
		// if there isn't move to the next dialogue object and try again
		++currentDialogue;
		return sayNext();
	}
}

void DialogueDisplay::update(Step * _step){
	Entity::update(_step);
	timeout->update(_step);
}