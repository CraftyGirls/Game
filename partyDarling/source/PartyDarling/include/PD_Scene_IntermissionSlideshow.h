#pragma once

#include <Scene_SlideShow.h>

class ComponentShaderText;

class PD_Scene_IntermissionSlideshow : public Scene_SlideShow{
public:
	PD_Scene_IntermissionSlideshow(Game * _game, int _plotPosition = 1);
	~PD_Scene_IntermissionSlideshow();

	virtual void update(Step * _step) override;
};