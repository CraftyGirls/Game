#pragma once

#include <PD_Furniture.h>
#include <PD_FurnitureParser.h>
#include <PD_FurnitureComponent.h>
#include <PD_FurnitureComponentDefinition.h>
#include <PD_ResourceManager.h>

#include <shader/Shader.h>
#include <NumberUtils.h>
#include <Easing.h>
#include <MeshDeformation.h>
#include <PD_Slot.h>

PD_Furniture::PD_Furniture(BulletWorld * _bulletWorld, PD_FurnitureDefinition * _def, Shader * _shader, Anchor_t _anchor) :
	RoomObject(_bulletWorld, new TriMesh(true), _shader, _anchor)
{
	// make sure that there's only one root
	assert(_def->components.size() == 1);

	// build the furniture
	PD_BuildResult buildResult = _def->components.at(0)->build();
	
	// get a texture for the furniture type
	Texture * tex = _def->textures.pop();
	tex->load();
	mesh->pushTexture2D(tex);
	mesh->setScaleMode(GL_NEAREST);

	// copy the furniture mesh into this entity's mesh
	mesh->insertVertices(*buildResult.mesh);
	// delete the temporary mesh
	delete buildResult.mesh;
	
	//Deformers
	float lowerFlareVal = sweet::NumberUtils::randomFloat(0.f,0.4f);
	float upperFlareVal = sweet::NumberUtils::randomFloat(0.f,(1.f - (0.5f+lowerFlareVal)));
	float lowerBoundVal = sweet::NumberUtils::randomFloat(0.2f,0.3f);
		
	if(_def->twist){
		MeshDeformation::twist(mesh, lowerFlareVal, upperFlareVal, lowerBoundVal, Easing::kEASE_IN_OUT_CUBIC);
	}
		//MeshDeformation::bend(mesh, lowerFlareVal, upperFlareVal, lowerBoundVal, Easing::kEASE_IN_OUT_CUBIC);
	if(_def->flare){
		MeshDeformation::flare(mesh, lowerFlareVal, upperFlareVal, lowerBoundVal, Easing::kEASE_IN_OUT_CUBIC);
	}

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
	createRigidBody(_def->mass * FURNITURE_MASS_SCALE );
	
	translatePhysical(glm::vec3(0, -boundingBox.y, 0.f), false);

	// Get type
	type = _def->type;

	// Get parent types
	parentTypes.insert(parentTypes.begin(), _def->parents.begin(), _def->parents.end());

	// Get the sides information
	if(_def->sides.front != PD_Side::kNONE){
		emptySlots[PD_Side::kFRONT] = new PD_Slot(_def->sides.front, 0.f, boundingBox.width);
	}
	if(_def->sides.back != PD_Side::kNONE){
		emptySlots[PD_Side::kBACK] = new PD_Slot(_def->sides.back, 0.f, boundingBox.width);
	}
	if(_def->sides.right != PD_Side::kNONE){
		emptySlots[PD_Side::kRIGHT] = new PD_Slot(_def->sides.right, 0.f, boundingBox.depth);
	}
	if(_def->sides.left != PD_Side::kNONE){
		emptySlots[PD_Side::kLEFT] = new PD_Slot(_def->sides.left, 0.f, boundingBox.depth);
	}
	if(_def->sides.top != PD_Side::kNONE){
		emptySlots[PD_Side::kTOP] = new PD_Slot(_def->sides.top, 0.f, boundingBox.width);
	}
}
