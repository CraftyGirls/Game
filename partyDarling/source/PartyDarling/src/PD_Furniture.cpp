#pragma once

#include <PD_Furniture.h>
#include <PD_FurnitureParser.h>
#include <PD_FurnitureComponent.h>
#include <shader/Shader.h>
#include <MeshEntity.h>
#include <PD_FurnitureComponentDefinition.h>
#include <PD_ResourceManager.h>

PD_Furniture::PD_Furniture(BulletWorld * _bulletWorld, PD_FurnitureDefinition * _def, Shader * _shader, Anchor_t _anchor) :
	RoomObject(_bulletWorld, new TriMesh(), _shader, _anchor)
{
	// make sure that there's only one root
	assert(_def->components.size() == 1);

	// build the furniture
	TriMesh * tempMesh = _def->components.at(0)->build();
	
	// copy the furniture mesh into this entity's mesh
	mesh->vertices.insert(mesh->vertices.end(), tempMesh->vertices.begin(), tempMesh->vertices.end());
	mesh->indices.insert(mesh->indices.end(), tempMesh->indices.begin(), tempMesh->indices.end());
	
	// delete the temporary mesh
	delete tempMesh;

	// move all of the vertices up so that the origin is at the base of the mesh
	/*float h = mesh->calcBoundingBox().height * 0.5f;
	for(Vertex & v : mesh->vertices){
		v.y += h;
	}*/

	// Make the mesh dirty since the verts have changed
	// May be redunant but do it as a safe guard
	mesh->dirty = true;
	
	meshTransform->scale(0.15f, 0.15f, 0.15f);
	freezeTransformation();

	// we need to inform the RoomObject of the new bounding box here
	boundingBox = mesh->calcBoundingBox();

	// create the bullet stuff
	setColliderAsBoundingBox();
	createRigidBody(0);
	
	//setTranslationPhysical(0, mesh->calcBoundingBox().height * 0.5f, 0.f);
}
