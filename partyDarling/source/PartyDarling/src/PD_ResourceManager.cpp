#pragma once

#include <PD_ResourceManager.h>

#include <PD_Assets.h>

Scenario * PD_ResourceManager::scenario = nullptr;
Scenario * PD_ResourceManager::itemTextures = nullptr;
Scenario * PD_ResourceManager::componentTextures = nullptr;
DatabaseConnection * PD_ResourceManager::db = nullptr;
std::vector<PD_FurnitureDefinition*> PD_ResourceManager::furnitureDefinitions;
PD_FurnitureComponentContainer * PD_ResourceManager::furnitureComponents = nullptr;

void PD_ResourceManager::init(){
	// register custom asset types
	Asset::registerType("item", &AssetItem::create);
	Asset::registerType("character", &AssetCharacter::create);
	Asset::registerType("room", &AssetRoom::create);

	// initialize assets
	scenario = new Scenario("assets/scenario.json");
	//itemTextures = new Scenario("assets/item-textures.json");
	//componentTextures = new Scenario("assets/component-textures.json");


	// parse furniture
	{
		Json::Value root;
		Json::Reader reader;
		std::string jsonLoaded = FileUtils::readFile("assets/furniture.json");
		bool parsingSuccessful = reader.parse( jsonLoaded, root );
		if(!parsingSuccessful){
			Log::error("JSON parse failed: " + reader.getFormattedErrorMessages()/* + "\n" + jsonLoaded*/);
		}else{
			for(auto furnDef : root["furniture"]) {
				furnitureDefinitions.push_back( new PD_FurnitureDefinition(furnDef));
			}
		}
	}
	furnitureComponents = new PD_FurnitureComponentContainer("assets/furniture.json");

	db = new DatabaseConnection("data/test.db");

	resources.push_back(scenario);
}

int PD_ResourceManager::dbCallback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

void PD_ResourceManager::testSql(std::string _sql, bool _async){
	if(_async){
		db->queryDbAsync(_sql, dbCallback);
	}else{
		db->queryDb(_sql, dbCallback);
	}
}