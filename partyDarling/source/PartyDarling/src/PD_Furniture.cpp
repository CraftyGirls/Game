#pragma once

#include <PD_Furniture.h>
#include <PD_FurnitureParser.h>
#include <PD_FurnitureComponent.h>
#include <shader/Shader.h>
#include <MeshEntity.h>
#include <PD_FurnitureComponentDefinition.h>
#include <PD_ResourceManager.h>
#include <NumberUtils.h>
#include <Easing.h>

#define FURNITURE_MASS_SCALE 0.05

PD_Furniture::PD_Furniture(BulletWorld * _bulletWorld, PD_FurnitureDefinition * _def, Shader * _shader, Anchor_t _anchor) :
	RoomObject(_bulletWorld, new TriMesh(true), _shader, _anchor)
{
	// make sure that there's only one root
	assert(_def->components.size() == 1);

	// build the furniture
	PD_BuildResult buildResult = _def->components.at(0)->build();
	
	// get a texture for the furniture type
	std::stringstream ss;
	ss << "assets/textures/furniture/" << _def->type << "_" << sweet::NumberUtils::randomInt(1, 2) << ".png";
	Texture * tex = new Texture(ss.str(), false, true, true);
	tex->load();
	mesh->pushTexture2D(tex);
	mesh->setScaleMode(GL_NEAREST);

	// copy the furniture mesh into this entity's mesh
	mesh->insertVertices(buildResult.mesh);
	
	//Deformers
	sweet::Box deformerBoundingBox = mesh->calcBoundingBox();
	
	
	float lowerFlareVal = sweet::NumberUtils::randomFloat(0.f,0.4f);
	float upperFlareVal = sweet::NumberUtils::randomFloat(0.f,(1.f - (0.5f+lowerFlareVal)));
	float lowerBoundVal = sweet::NumberUtils::randomFloat(0.2f,0.3f);

	for(auto & v : mesh->vertices){
		//normalize vertex position by the bounding box of the mesh
		float vertX = (v.x) / deformerBoundingBox.width;
		float vertY = (v.y - deformerBoundingBox.y - lowerBoundVal) / deformerBoundingBox.height;
		float vertZ = (v.z) / deformerBoundingBox.depth;

		//create a vec4 of vertex position
		glm::vec4 vertVector4 = glm::vec4( vertX, vertY, vertZ, 1.0f);
		//flare deformation matrix
		glm::mat4 flareMatrix =  glm::mat4 (Easing::easeInOutCubic(vertY,0.5f+lowerFlareVal,0.5f+upperFlareVal,1.f),0.f,0.f,0.f,
											0.f,1.f,0.f,0.f,
											0.f,0.f,Easing::easeInOutCubic(vertY,0.5f+lowerFlareVal,0.5f+upperFlareVal,1.f),0.f,
											0.f,0.f,0.f,1.f);

		//bend deformer matrix
		glm::mat4 bendMatrix = glm::mat4 (glm::cos(0.25f*vertY),glm::sin(0.25f*vertY),0.f,0.f,
											Easing::easeInOutCubic(vertY,0.5f,0.1f,1.f),Easing::easeInOutCubic(vertY,0.5f,0.1f,1.f),0.f,0.f,
											0.f,0.f,1.f,0.f,
											0.f,0.f,0.f,1.f);

		//twist deformer matrix
		glm::mat4 twistMatrix = glm::mat4 (glm::cos(0.25f*vertY),0.f,glm::sin(0.25f*vertY),0.f,
											0.f,1.f,0.f,0.f,
											-1.0f*(glm::sin(0.25f*vertY)),0.f,glm::cos(0.25f*vertY),0.f,
											0.f,0.f,0.f,1.f);

		//multiply matrix by normalize vertex position vector
		glm::vec4 newVertVector4 = flareMatrix /*bendMatrix*/ * twistMatrix * vertVector4;

		//change vertex positions, scale them up by bounding box dimensions
		v.x = newVertVector4.x  * deformerBoundingBox.width;
		v.y = newVertVector4.y * deformerBoundingBox.height + deformerBoundingBox.y + lowerBoundVal;
		v.z = newVertVector4.z * deformerBoundingBox.depth;


	}

	// delete the temporary mesh
	delete buildResult.mesh;
	/**** Won't work, since I guess the stuff over the origin won't necessarily be the same height as the stuf below???? *****
	// move all of the vertices up so that the origin is at the base of the mesh
	float h = mesh->calcBoundingBox().height * 0.5f;
	for(Vertex & v : mesh->vertices){
		v.y += h;
	}
	*/

	// Make the mesh dirty since the verts have changed
	// May be redunant but do it as a safe guard
	mesh->dirty = true;
	
	meshTransform->scale(0.15f, 0.15f, 0.15f);
	freezeTransformation();

	// we need to inform the RoomObject of the new bounding box here
	boundingBox = mesh->calcBoundingBox();


	// create the bullet stuff
	if(_def->detailedCollider){
		shape = buildResult.collider;
	}else{
		delete buildResult.collider;
		setColliderAsBoundingBox();
	}
	createRigidBody(_def->mass * FURNITURE_MASS_SCALE);
	
	translatePhysical(glm::vec3(0, mesh->calcBoundingBox().height * 0.5f, 0.f), false);
}
