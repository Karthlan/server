//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// a Tile represents a single field on the map.
// it is a list of Items
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////


#include "definitions.h"

#include <string>
#include <iostream>

#include "tile.h"
#include "game.h"
#include "player.h"
#include "creature.h"

extern Game g_game;

ReturnValue Tile::isBlocking(int objectstate, bool ignoreCreature /*= false*/, bool ignoreMoveableBlocking /*= false*/) const
{
	if(isPz() && ((objectstate & BLOCK_PZ) == BLOCK_PZ)) {
		return RET_PROTECTIONZONE;
	}

	if(((objectstate & BLOCK_PATHFIND) == BLOCK_PATHFIND) && (floorChange() || getTeleportItem())) {
		return RET_THEREISNOWAY;
	}
		
	if(ground) {
		const ItemType& groundType = Item::items[ground->getID()];

		if(((objectstate & BLOCK_PROJECTILE) == BLOCK_PROJECTILE) &&
			groundType.blockProjectile)
			return RET_CANNOTTHROW;

		//if((groundType.blockPathFind || groundType.blockSolid) && ((objectstate & BLOCK_PATHFIND) == BLOCK_PATHFIND))
		//	return RET_THEREISNOWAY;

		if(((objectstate & BLOCK_PICKUPABLE) == BLOCK_PICKUPABLE)) {			
			if(groundType.blockSolid && (!groundType.hasHeight || groundType.pickupable))
				return RET_NOTENOUGHROOM;
		}

		if(((objectstate & BLOCK_SOLID) == BLOCK_SOLID) && groundType.blockSolid)
			return RET_NOTENOUGHROOM;
	}
	else if( !((objectstate & BLOCK_PROJECTILE) == BLOCK_PROJECTILE)) {
		return RET_NOTILE;
	}

	if(!ignoreCreature && !creatures.empty() && ((objectstate & BLOCK_SOLID) == BLOCK_SOLID))
		return RET_CREATUREBLOCK;

	ItemVector::const_iterator iit;
	for (iit = topItems.begin(); iit != topItems.end(); ++iit) {
		const ItemType& iiType = Item::items[(*iit)->getID()];

		if(((objectstate & BLOCK_PROJECTILE) == BLOCK_PROJECTILE)) {
			if(iiType.blockProjectile)
				return RET_CANNOTTHROW;
			//else
			//	continue;
		}

		//if((iiType.blockPathFind || iiType.blockSolid) && ((objectstate & BLOCK_PATHFIND) == BLOCK_PATHFIND) &&
		//	!(ignoreMoveableBlocking && iiType.moveable))
		//	return RET_THEREISNOWAY;
		//

		if(((objectstate & BLOCK_PICKUPABLE) == BLOCK_PICKUPABLE)) {			
			if(iiType.blockSolid && (!iiType.hasHeight || iiType.pickupable))
				return RET_NOTENOUGHROOM;
		}

		if(((objectstate & BLOCK_SOLID) == BLOCK_SOLID) && iiType.blockSolid &&
			!(ignoreMoveableBlocking && iiType.moveable))
			return RET_NOTENOUGHROOM;
	}
	
	for (iit = downItems.begin(); iit != downItems.end(); ++iit) {
		const ItemType& iiType = Item::items[(*iit)->getID()];

		if(((objectstate & BLOCK_PROJECTILE) == BLOCK_PROJECTILE)) {
			if(iiType.blockProjectile)
				return RET_CANNOTTHROW;
			//else
			//	continue;
		}

		//if((iiType.blockPathFind || iiType.blockSolid) && ((objectstate & BLOCK_PATHFIND) == BLOCK_PATHFIND) &&
		//	!(ignoreMoveableBlocking && iiType.moveable))
		//	return RET_THEREISNOWAY;

		if(((objectstate & BLOCK_PICKUPABLE) == BLOCK_PICKUPABLE)) {
			if(iiType.blockSolid && (!iiType.hasHeight || iiType.pickupable))
				return RET_NOTENOUGHROOM;
		}

		if(((objectstate & BLOCK_SOLID) == BLOCK_SOLID) && iiType.blockSolid &&
			!(ignoreMoveableBlocking && iiType.moveable))
			return RET_NOTENOUGHROOM;
	}

	return RET_NOERROR;
}


bool Tile::isBlockingProjectile() const
{
	if(ground){
		const ItemType& groundType = Item::items[ground->getID()];

		if(groundType.blockProjectile)
			return true;
	}

	ItemVector::const_iterator iit;
	for(iit = topItems.begin(); iit != topItems.end(); ++iit){
		const ItemType& iiType = Item::items[(*iit)->getID()];

		if(iiType.blockProjectile)
			return true;
	}
	
	for(iit = downItems.begin(); iit != downItems.end(); ++iit){
		const ItemType& iiType = Item::items[(*iit)->getID()];

		if(iiType.blockProjectile)
			return true;
	}

	return false;
}

bool Tile::floorChange() const
{
  ItemVector::const_iterator iit;
  if(ground && ground->floorChangeDown())
    return true;

  for (iit = topItems.begin(); iit != topItems.end(); ++iit){  
		if ((*iit)->floorChangeNorth() || (*iit)->floorChangeSouth() || (*iit)->floorChangeEast() || (*iit)->floorChangeWest())
		return true;      
	}

  for (iit = downItems.begin(); iit != downItems.end(); ++iit){ 
		if ((*iit)->floorChangeNorth() || (*iit)->floorChangeSouth() || (*iit)->floorChangeEast() || (*iit)->floorChangeWest())
			return true;
	}

  return false;
}

bool Tile::floorChangeDown() const
{
	if(ground && ground->floorChangeDown())
		return true;
		
	ItemVector::const_iterator iit;
	for(iit = topItems.begin(); iit != topItems.end(); ++iit){
		if((*iit)->floorChangeDown())
			return true;
	}

  	for(iit = downItems.begin(); iit != downItems.end(); ++iit){
		if((*iit)->floorChangeDown())
			return true;
	}
	
  	return false;
}

bool Tile::floorChange(Direction direction) const
{  
	ItemVector::const_iterator iit;
	for(iit = topItems.begin(); iit != topItems.end(); ++iit){
		if(direction == NORTH){  
			if((*iit)->floorChangeNorth())
				return true;
		}
		else if(direction == SOUTH){
			if ((*iit)->floorChangeSouth())
			return true;
		}
		else if(direction == EAST){
			if ((*iit)->floorChangeEast())
			return true;
		}
		else if(direction == WEST){
			if ((*iit)->floorChangeWest())
			return true;
		}
	}

	for(iit = downItems.begin(); iit != downItems.end(); ++iit){
		if(direction == NORTH){  
			if ((*iit)->floorChangeNorth())
			return true;
		}
		else if(direction == SOUTH){
			if ((*iit)->floorChangeSouth())
			return true;
		}
		else if(direction == EAST){
			if ((*iit)->floorChangeEast())
			return true;
		}
		else if(direction == WEST){
			if ((*iit)->floorChangeWest())
			return true;
		}
	}

 	return false;
}


int Tile::getThingCount() const
{
	return (uint32_t) (ground ? 1 : 0) + (splash ? 1 : 0) + downItems.size()+ topItems.size() + creatures.size();
}

std::string Tile::getDescription(uint32_t lookDistance) const
{
  std::string ret = "You dont know why, but you cant see anything!";
	return ret;
}

Thing* Tile::getTopMoveableThing()
{	
	if(ground && !ground->isNotMoveable())
		return ground;
    
	if(splash && !splash->isNotMoveable())
		return splash;

	for(int i = 0; i < topItems.size(); i++){
		if(topItems[i] && !topItems[i]->isNotMoveable())
			return topItems[i];
	}

	for(int i = 0; i < creatures.size(); i++){
		return creatures[i];
	}

	for(int i = 0; i < downItems.size(); i++){
		if(downItems[i] && !downItems[i]->isNotMoveable())
			return downItems[i];
	}

	return NULL;
}

Teleport* Tile::getTeleportItem() const
{
  Teleport* teleport = NULL;
  for (ItemVector::const_iterator iit = topItems.begin(); iit != topItems.end(); ++iit){
		teleport = dynamic_cast<Teleport*>(*iit);
		if(teleport)
			return teleport;
  }

	return NULL;
}

MagicEffectItem* Tile::getFieldItem()
{
  MagicEffectItem* fieldItem = NULL;
  for(ItemVector::const_iterator iit = downItems.begin(); iit != downItems.end(); ++iit){
		fieldItem = dynamic_cast<MagicEffectItem*>(*iit);
		if(fieldItem)
			return fieldItem;
  }

	return NULL;
}

Creature* Tile::getTopCreature()
{
	if(creatures.begin() != creatures.end()){
		return *(creatures.begin());
	}

	return NULL;
}

Item* Tile::getTopDownItem()
{
	if(downItems.begin() != downItems.end()){
		return *(downItems.begin());
	}

	return NULL;
}

Item* Tile::getTopTopItem()
{
	if(topItems.begin() != topItems.end()){
		return *(topItems.begin());
	}

	return NULL;
}

Thing* Tile::getTopThing()
{
	Thing* thing = NULL;
	thing = getTopCreature();
	if(thing != NULL)
		return thing;

	thing = getTopDownItem();
	if(thing != NULL)
		return thing;

	thing = getTopTopItem();
	if(thing != NULL)
		return thing;

	if(splash)
		return splash;

	if(ground)
		return ground;

	return NULL;
}

Item* Tile::getMoveableBlockingItem()
{
	for(ItemVector::const_iterator iit = downItems.begin(); iit != downItems.end(); ++iit){
		const ItemType& iiType = Item::items[(*iit)->getID()];
		if((iiType.blockPathFind || iiType.blockSolid) && iiType.moveable)
			return *iit;
	}

	return NULL;
}

void Tile::addThing(Thing *thing)
{
	thing->setParent(this);

	Creature* creature = dynamic_cast<Creature*>(thing);
	if (creature) {
    	creatures.insert(creatures.begin(), creature);
  }
  else
  {
    Item *item = dynamic_cast<Item*>(thing);
		if(!item)
			return;

    if (item->isGroundTile())
    {
      ground = item;
    }
    else if(item->isSplash()){
		if(splash == NULL){
			splash = item;
		}
		else{
			//Should not add the splash directly
			//use game->addthing method
		}
	}
    else if (item->isAlwaysOnTop())
    {
      topItems.insert(topItems.begin(), item);
    }
    else
    {
      downItems.insert(downItems.begin(), item);
    }
  }
}

bool Tile::isPz() const
{
  return pz;
}

void Tile::setPz()
{
  pz = true;
}

ReturnValue Tile::__queryMaxCount(int32_t index, const Thing* thing, uint32_t count,
	uint32_t& maxQueryCount) const
{
	maxQueryCount = std::max((uint32_t)1, count);
	return RET_NOERROR;
}

ReturnValue Tile::__queryAdd(uint32_t index, const Thing* thing, uint32_t count) const
{
	//If its a new (summoned item) always accept it
	if(thing->getParent() == NULL){
		return RET_NOERROR;
	}

	Thing* iithing = NULL;

	if(dynamic_cast<const Creature*>(thing)){
		if(!creatures.empty())
			return RET_NOTPOSSIBLE;

		for(uint32_t i = 0; i < getThingCount(); ++i){
			iithing = __getThing(i);

			if(const Item* iitem = dynamic_cast<const Item*>(iithing)){
				const ItemType& iiType = Item::items[iitem->getID()];

				if(iiType.blockSolid)
					return RET_NOTPOSSIBLE;
			}
		}
	}
	else if(const Item* item = dynamic_cast<const Item*>(thing)){
		if(!creatures.empty() && item->isBlocking())
			return RET_NOTENOUGHROOM;

		for(uint32_t i = 0; i < getThingCount(); ++i){
			iithing = __getThing(i);

			if(const Item* iitem = dynamic_cast<const Item*>(iithing)){
				const ItemType& iiType = Item::items[iitem->getID()];

				if(item->isPickupable()){
					if(iiType.blockSolid && (!iiType.hasHeight || iiType.pickupable))
						return RET_NOTENOUGHROOM;
				}

				if(item->isBlocking() && iiType.blockSolid)
					return RET_NOTENOUGHROOM;
			}
		}
	}

	return RET_NOERROR;
}

ReturnValue Tile::__queryRemove(const Thing* thing, uint32_t count) const
{
	uint32_t index = __getIndexOfThing(thing);

	if(index == -1){
		return RET_NOTPOSSIBLE;
	}
	
	const Item* item = dynamic_cast<const Item*>(thing);
	if(item == NULL){
		return RET_NOTPOSSIBLE;
	}
	
	if(item->isNotMoveable()){
		return RET_NOTMOVEABLE;
	}

	if(item->isStackable() && (count == 0 || count > item->getItemCountOrSubtype())){
		return RET_NOTPOSSIBLE;
	}

	return RET_NOERROR;
}

Cylinder* Tile::__queryDestination(int32_t& index, const Thing* thing, Thing** destThing)
{
	Teleport* teleport = getTeleportItem();

	Tile* destTile = this;
	*destThing = NULL;

	if(teleport){
		destTile = g_game.getTile(teleport->getDestPos().x, teleport->getDestPos().y, teleport->getDestPos().z);
	}
	else{
		if(destTile->floorChange()){
			if(destTile->floorChange(NORTH) && destTile->floorChange(EAST)){
				destTile = g_game.getTile(getTilePosition().x + 1, getTilePosition().y - 1, getTilePosition().z - 1);
			}
			else if(destTile->floorChange(NORTH) && destTile->floorChange(WEST)){
				destTile = g_game.getTile(getTilePosition().x - 1, getTilePosition().y - 1, getTilePosition().z - 1);
			}
			else if(destTile->floorChange(SOUTH) && destTile->floorChange(EAST)){
				destTile = g_game.getTile(getTilePosition().x + 1, getTilePosition().y + 1, getTilePosition().z - 1);
			}
			else if(destTile->floorChange(SOUTH) && destTile->floorChange(WEST)){
				destTile = g_game.getTile(getTilePosition().x - 1, getTilePosition().y + 1, getTilePosition().z - 1);
			}
			else if(destTile->floorChange(NORTH)){
				destTile = g_game.getTile(getTilePosition().x, getTilePosition().y - 1, getTilePosition().z - 1);
			}
			else if(destTile->floorChange(SOUTH)){
				destTile = g_game.getTile(getTilePosition().x, getTilePosition().y + 1, getTilePosition().z - 1);
			}
			else if(destTile->floorChange(EAST)){
				destTile = g_game.getTile(getTilePosition().x + 1, getTilePosition().y, getTilePosition().z - 1);
			}
			else if(destTile->floorChange(WEST)){
				destTile = g_game.getTile(getTilePosition().x - 1, getTilePosition().y, getTilePosition().z - 1);
			}                                      
		}

		if(destTile->floorChangeDown()){
			destTile = g_game.getTile(getTilePosition().x, getTilePosition().y, getTilePosition().z + 1);

			if(destTile == NULL){
				return this;
			}

			if(destTile->floorChange(NORTH) && destTile->floorChange(EAST)){
				destTile = g_game.getTile(getTilePosition().x - 2, getTilePosition().y + 2, getTilePosition().z + 1);
			}
			else if(destTile->floorChange(NORTH) && destTile->floorChange(WEST)){
				destTile = g_game.getTile(getTilePosition().x + 2, getTilePosition().y + 2, getTilePosition().z + 1);
			}
			else if(destTile->floorChange(SOUTH) && destTile->floorChange(EAST)){
				destTile = g_game.getTile(getTilePosition().x - 2, getTilePosition().y - 2, getTilePosition().z + 1);
			}
			else if(destTile->floorChange(SOUTH) && destTile->floorChange(WEST)){
				destTile = g_game.getTile(getTilePosition().x + 2, getTilePosition().y - 2, getTilePosition().z + 1);
			}
			else if(destTile->floorChange(NORTH)){
				destTile = g_game.getTile(getTilePosition().x, getTilePosition().y + 1, getTilePosition().z + 1);
			}
			else if(destTile->floorChange(SOUTH)){
				destTile = g_game.getTile(getTilePosition().x, getTilePosition().y - 1, getTilePosition().z + 1);
			}
			else if(destTile->floorChange(EAST)){
				destTile = g_game.getTile(getTilePosition().x - 1, getTilePosition().y, getTilePosition().z + 1);
			}
			else if(destTile->floorChange(WEST)){
				destTile = g_game.getTile(getTilePosition().x + 1, getTilePosition().y, getTilePosition().z + 1);
			}
		}
	}

	*destThing = destTile->getTopDownItem();
	return destTile;
}

ReturnValue Tile::__addThing(Thing* thing)
{
	return __addThing(0, thing);
}

ReturnValue Tile::__addThing(uint32_t index, Thing* thing)
{
	thing->setParent(this);

	Creature* creature = dynamic_cast<Creature*>(thing);
	if(creature){
		creatures.insert(creatures.begin(), creature);
		return RET_NOERROR;
  }
  else{
    Item* item = dynamic_cast<Item*>(thing);

		if(item == NULL)
			return RET_NOTPOSSIBLE;

		const Position& cylinderMapPos = getPosition();

		SpectatorVec list;
		SpectatorVec::iterator it;
		g_game.getSpectators(Range(cylinderMapPos, true), list);

    if(item->isGroundTile()){
			if(ground == NULL){
				//send to client
				for(it = list.begin(); it != list.end(); ++it) {
					(*it)->onAddTileItem(cylinderMapPos, item);
				}
			}
			else{
				uint32_t index = __getIndexOfThing(ground);

				//send to client
				for(it = list.begin(); it != list.end(); ++it) {
					(*it)->onUpdateTileItem(cylinderMapPos, index, ground, item);
				}
			}

			ground = item;
    }
    else if(item->isSplash()){
			if(splash == NULL){
				//send to client
				for(it = list.begin(); it != list.end(); ++it) {
					(*it)->onAddTileItem(cylinderMapPos, item);
				}
			}
			else{
				uint32_t index = __getIndexOfThing(splash);

				//send to client
				for(it = list.begin(); it != list.end(); ++it) {
					(*it)->onUpdateTileItem(cylinderMapPos, index, splash, item);
				}
			}

			splash = item;
		}
		else if(item->isAlwaysOnTop()){
			topItems.insert(topItems.begin(), item);

			//send to client
			for(it = list.begin(); it != list.end(); ++it) {
				(*it)->onAddTileItem(cylinderMapPos, item);
			}
		}
		else{
			downItems.insert(downItems.begin(), item);

			//send to client
			for(it = list.begin(); it != list.end(); ++it) {
				(*it)->onAddTileItem(cylinderMapPos, item);
			}

			return RET_NOERROR;
		}
	}

	return RET_NOTPOSSIBLE;
}

ReturnValue Tile::__updateThing(Thing* thing, uint32_t count)
{
	int32_t index = __getIndexOfThing(thing);
	if(index == -1){
#ifdef __DEBUG__
		std::cout << "Failure: [Tile::__updateThing] index == -1" << std::endl;
#endif
		return RET_NOTPOSSIBLE;
	}

	Item* item = dynamic_cast<Item*>(thing);
	if(item == NULL){
		return RET_NOTPOSSIBLE;
	}

	item->setItemCountOrSubtype(count);

	const Position& cylinderMapPos = getPosition();

	SpectatorVec list;
	SpectatorVec::iterator it;
	g_game.getSpectators(Range(cylinderMapPos, true), list);

	//send to client
	for(it = list.begin(); it != list.end(); ++it) {
		(*it)->onUpdateTileItem(cylinderMapPos, index, item, item);
	}

	return RET_NOERROR;
}

ReturnValue Tile::__updateThing(uint32_t index, Thing* thing)
{
	int32_t pos = index;

	Item* item = dynamic_cast<Item*>(thing);
	if(item == NULL){
		return RET_NOTPOSSIBLE;
	}

	Item* oldItem = NULL;

	if(pos == 0 && ground){
		oldItem = ground;
		ground = item;
	}

  --pos;

	if(pos == 0 && splash){
		oldItem = splash;
		splash = item;
	}

	--pos;

	if(pos >= 0 && pos < topItems.size()){
		ItemVector::iterator it = topItems.begin();
		it += pos;
		pos = 0;
		
		oldItem = (*it);
		topItems.insert(it, item);
		topItems.erase(it);
	}

	pos -= (uint32_t)topItems.size();

	if(pos >= 0 && pos < creatures.size()){
		return RET_NOTPOSSIBLE;
	}

  pos -= (uint32_t)creatures.size();

	if(pos >= 0 && pos < downItems.size()){
		ItemVector::iterator it = downItems.begin();
		it += pos;
		pos = 0;

		oldItem = (*it);
		downItems.insert(it, item);
		topItems.erase(it);
	}

	if(pos == 0){
		item->setParent(this);

		const Position& cylinderMapPos = getPosition();

		SpectatorVec list;
		SpectatorVec::iterator it;
		g_game.getSpectators(Range(cylinderMapPos, true), list);

		//send to client
		for(it = list.begin(); it != list.end(); ++it) {
			(*it)->onUpdateTileItem(cylinderMapPos, index, oldItem, item);
		}

		return RET_NOERROR;
	}

	return RET_NOTPOSSIBLE;
}

ReturnValue Tile::__removeThing(Thing* thing, uint32_t count)
{
	Creature* creature = dynamic_cast<Creature*>(thing);
	if(creature){
    CreatureVector::iterator it;
		for(it = creatures.begin(); it != creatures.end(); ++it){
      if(*it == thing){
        creatures.erase(it);
        return RET_NOERROR;
      }
		}
	}
	else{
		Item *item = dynamic_cast<Item*>(thing);
		if(item == NULL)
			return RET_NOTPOSSIBLE;

		uint32_t index = __getIndexOfThing(item);
		if(index == -1){
	#ifdef __DEBUG__
			std::cout << "Failure: [Tile::__removeThing] index == -1" << std::endl;
	#endif
			return RET_NOTPOSSIBLE;
		}

		const Position& cylinderMapPos = getPosition();

		SpectatorVec list;
		SpectatorVec::iterator it;
		g_game.getSpectators(Range(cylinderMapPos, true), list);

		if(item == ground){

			//send to client
			for(it = list.begin(); it != list.end(); ++it) {
				(*it)->onRemoveTileItem(cylinderMapPos, index, item);
			}

			ground->setParent(NULL);
			ground = NULL;
			return RET_NOERROR;
		}

		if(item == splash){
			//send to client
			for(it = list.begin(); it != list.end(); ++it) {
				(*it)->onRemoveTileItem(cylinderMapPos, index, item);
			}

			splash->setParent(NULL);
			splash = NULL;
			return RET_NOERROR;
		}

		ItemVector::iterator iit;
		if(item->isAlwaysOnTop()){
			for(iit = topItems.begin(); iit != topItems.end(); ++iit){
				if(*iit == item){
					//send to client
					for(it = list.begin(); it != list.end(); ++it) {
						(*it)->onRemoveTileItem(cylinderMapPos, index, item);
					}

					(*iit)->setParent(NULL);
					topItems.erase(iit);
					return RET_NOERROR;
				}
			}
		}
		else{
			for (iit = downItems.begin(); iit != downItems.end(); ++iit)
				if(*iit == item){
					if(item->isStackable() && count != item->getItemCountOrSubtype()){							
						item->setItemCountOrSubtype(item->getItemCountOrSubtype() - count);

						//send to client
						for(it = list.begin(); it != list.end(); ++it) {
							(*it)->onUpdateTileItem(cylinderMapPos, index, item, item);
						}
					}
					else {
						//send to client
						for(it = list.begin(); it != list.end(); ++it) {
							(*it)->onRemoveTileItem(cylinderMapPos, index, item);
						}

						(*iit)->setParent(NULL);
						downItems.erase(iit);
					}

					return RET_NOERROR;
				}
		}
  }

  return RET_NOTPOSSIBLE;
}

int32_t Tile::__getIndexOfThing(const Thing* thing) const
{
	int n = 0;
  
	if(ground){
		if(ground == thing){
			return 0;
		}
  }
  
  if(splash){
    if(thing == splash)
      return 1;

    ++n;
  }

  ItemVector::const_iterator iit;
  for(iit = topItems.begin(); iit != topItems.end(); ++iit){
    ++n;
    if((*iit) == thing)
      return n;
  }

	CreatureVector::const_iterator cit;
	for(cit = creatures.begin(); cit != creatures.end(); ++cit){
		++n;
		if((*cit) == thing)
			return n;
	}

  for(iit = downItems.begin(); iit != downItems.end(); ++iit){
    ++n;
    if((*iit) == thing)
      return n;
  }

	return -1;
}

Thing* Tile::__getThing(uint32_t index) const
{
  if(index == 0)
    return ground;

  --index;

  if(splash){
    if(index == 0)
      return splash;
    --index;
  }

  if((unsigned) index < topItems.size())
    return topItems[index];

  index -= (uint32_t)topItems.size();

  if((unsigned) index < creatures.size())
    return creatures[index];

  index -= (uint32_t)creatures.size();

  if((unsigned) index < downItems.size())
    return downItems[index];

  return NULL;
}

Thing* Tile::__getThing(uint32_t index)
{
  if(index == 0)
    return ground;

  --index;

  if(splash){
    if(index == 0)
      return splash;
    --index;
  }

  if((unsigned) index < topItems.size())
    return topItems[index];

  index -= (uint32_t)topItems.size();

  if((unsigned) index < creatures.size())
    return creatures[index];

  index -= (uint32_t)creatures.size();

  if((unsigned) index < downItems.size())
    return downItems[index];

  return NULL;
}


void Tile::postAddNotification(const Thing* thing, bool hasOwnership /*= true*/)
{
	const Position& cylinderMapPos = getPosition();

	SpectatorVec list;
	SpectatorVec::iterator it;
	g_game.getSpectators(Range(cylinderMapPos, true), list);

	for(it = list.begin(); it != list.end(); ++it){
		if(Player* player = dynamic_cast<Player*>(*it)){
			player->postAddNotification(thing, false);
		}
	}
}

void Tile::postRemoveNotification(const Thing* thing, bool hadOwnership /*= true*/)
{
	const Position& cylinderMapPos = getPosition();

	SpectatorVec list;
	SpectatorVec::iterator it;
	g_game.getSpectators(Range(cylinderMapPos, true), list);

	if(getThingCount() > 8){
		//send to client
		for(it = list.begin(); it != list.end(); ++it){
			(*it)->onUpdateTile(cylinderMapPos);
		}
	}

	for(it = list.begin(); it != list.end(); ++it){
		if(Player* player = dynamic_cast<Player*>(*it)){
			player->postRemoveNotification(thing, false);
		}
	}
}

void Tile::__internalAddThing(Thing* thing)
{
	//
}

void Tile::__internalAddThing(uint32_t index, Thing* thing)
{
	//
}
