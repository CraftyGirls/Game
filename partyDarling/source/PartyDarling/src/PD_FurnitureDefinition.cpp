#pragma once

#include <PD_FurnitureDefinition.h>
#include <PD_FurnitureComponentDefinition.h>

PD_FurnitureDefinition::PD_FurnitureDefinition(Json::Value _jsonDef) :
	type(_jsonDef.get("type", "UNDEFINED").asString()),
	mass(_jsonDef.get("mass", 0.f).asFloat()),
	detailedCollider(_jsonDef.get("detailedCollider", true).asBool())
{
	for(auto outCompJson : _jsonDef["components"]) {
		components.push_back(new PD_FurnitureComponentDefinition(outCompJson));								
	}
}
