#pragma once

#include <ResourceManager.h>
#include <SoundManager.h>
#include <Texture.h>

class PD_ResourceManager : public ResourceManager{
public:
	static Texture * crosshair;
	static Texture * cursor;
	static Texture * cheryl;
	static Texture * uvs;

	static void init();
};