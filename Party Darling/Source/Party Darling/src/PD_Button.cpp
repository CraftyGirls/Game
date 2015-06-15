#pragma once

#include <PD_Button.h>
#include <Font.h>
#include <shader\ComponentShaderBase.h>
#include <shader\ShaderComponentText.h>
#include <shader/ShaderComponentTexture.h>
#include <MeshFactory.h>

PD_Button::PD_Button(BulletWorld * _world, Scene * _scene) :
	NodeUI(_world, _scene),
	NodeBulletBody(_world),
	Entity()
{
	ComponentShaderBase * textShader = new ComponentShaderBase(true);
	textShader->addComponent(new ShaderComponentText(textShader));
	textShader->compileShader();

	float size = 30.f;
	Font * font = new Font("../assets/arial.ttf", size, false);
	//size = 1.f/size;
	normalLabel = new TextLabel(_world, _scene, font, textShader, 200);
	normalLabel->setText(L"normal");
	childTransform->addChild(normalLabel);//->scale(size/64.f);
	
	downLabel = new TextLabel(_world, _scene, font, textShader, 200);
	downLabel->setText(L"down");
	childTransform->addChild(downLabel);//->scale(size/64.f);
	
	overLabel = new TextLabel(_world, _scene, font, textShader, 200);
	overLabel->setText(L"over");
	childTransform->addChild(overLabel);//->scale(size/64.f);
	
	setWidth(200.f);
	updateCollider();
}

void PD_Button::update(Step * _step){
	NodeUI::update(_step);

	if(isHovered){
		if(isDown){
			normalLabel->setVisible(false);
			overLabel->setVisible(false);
			downLabel->setVisible(true);
		}else{
			normalLabel->setVisible(false);
			overLabel->setVisible(true);
			downLabel->setVisible(false);
		}
	}else{
		normalLabel->setVisible(true);
		overLabel->setVisible(false);
		downLabel->setVisible(false);
	}
}