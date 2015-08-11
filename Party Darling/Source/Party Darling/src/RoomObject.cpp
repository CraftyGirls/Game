#include <RoomObject.h>
#include <MeshInterface.h>
#include <Box.h>

Slot::Slot(float _loc, float _length) :
	loc(_loc),
	length(_length)
{
}

RoomObject::RoomObject(BulletWorld * _world, MeshInterface * _mesh, Anchor_t _anchor):
	BulletMeshEntity(_world, _mesh),
	anchor(_anchor),
	boundingBox(mesh->calcBoundingBox())
{
	 
}

RoomObject::~RoomObject(void){
}

void RoomObject::addComponent(RoomObject * obj){
	childTransform->addChild(obj);
	components.push_back(obj);
}

void RoomObject::setShader(Shader * _shader, bool _default){
	MeshEntity::setShader(_shader, _default);
	for(unsigned int i = 0; i < components.size(); ++i){
		RoomObject * obj = components.at(i);
		obj->setShader(_shader, _default);
	}
}

void RoomObject::translatePhysical(glm::vec3 _v, bool _relative){
	setTranslationPhysical(_v, _relative);
	for(unsigned int i = 0; i < components.size(); ++i){
		components.at(i)->translatePhysical(_v);
	}
}