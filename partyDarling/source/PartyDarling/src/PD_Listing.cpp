#pragma once

#include <PD_Listing.h>

std::map<Scenario *, PD_Listing *> PD_Listing::listings;

PD_Listing::PD_Listing(Scenario * _scenario){
	listings[_scenario] = this;
}

void PD_Listing::addCharacter(Person * _person){
	characters[_person->definition->id] = _person;
}
void PD_Listing::addItem(PD_Item * _item){
	items[_item->definition->id] = _item;
}
void PD_Listing::addRoom(Room * _room){
	rooms[_room->definition->id] = _room;
}