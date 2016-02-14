#pragma once

#include <PD_Item.h>

class PD_Door : public PD_Item{
public:
	enum Door_t{
		kNORTH,
		kSOUTH,
		kEAST,
		kWEST
	} side;

	PD_Door(BulletWorld * _world, Shader * _shader, Door_t _side, unsigned long int _doorIndex);

	void triggerInteract() override;
};