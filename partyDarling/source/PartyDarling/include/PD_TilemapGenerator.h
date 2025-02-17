#pragma once

#include <ProgrammaticTexture.h>
#include <MeshInterface.h>

class PD_TilemapGenerator : public ProgrammaticTexture{
public:

	unsigned long int max, pixelIncrement;
	void configure(unsigned long int _max = 255, unsigned long int _pixelIncrement = 255);

	PD_TilemapGenerator(unsigned long int _width, unsigned long int _height, bool _autorelease);

	void generateTilemap();

	MeshInterface * march(unsigned long int _thresh, bool _smooth);
private:
	unsigned long int thresh;
};