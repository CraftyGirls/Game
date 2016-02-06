#pragma once

#include <Room.h>
#include <math.h>

#include <MeshInterface.h>
#include <BulletMeshEntity.h>

#include <Sprite.h>
#include <Texture.h>
#include <RoomObject.h>
#include <RoomBuilder.h>
#include <PD_TilemapGenerator.h>

Room::Room(BulletWorld * _world, Shader * _shader) :
	BulletMeshEntity(_world, new QuadMesh(true), _shader),
	tilemapSprite(new Sprite())
{
	mesh->setScaleMode(GL_NEAREST);
}

void Room::addComponent(RoomObject * obj){
	components.push_back(obj);
}

void Room::setShader(Shader * _shader, bool _default){
	printHierarchy();
	BulletMeshEntity::setShader(_shader, _default);
	tilemapSprite->setShader(_shader, _default);
	for(unsigned int i = 0; i < components.size(); ++i){
		components.at(i)->setShader(_shader, _default);
	}
}

void Room::translatePhysical(glm::vec3 _v, bool _relative){
	BulletMeshEntity::translatePhysical(_v, _relative);
	tilemapSprite->meshTransform->translate(_v, _relative);
	//TODO: move room to an absolute position while moving components relatively? (maintain component spacing)
	for(unsigned int i = 0; i < components.size(); ++i){
		components.at(i)->translatePhysical(_v, _relative);
	}
}

Room::~Room(void){
}

glm::vec3 Room::getCenter() const{
	return glm::vec3((tilemap->width/2.f) * ROOM_TILE, 0.f, (tilemap->height/2.f) * ROOM_TILE);
}