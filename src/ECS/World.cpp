#include <ECS/World.hpp>

#include <cassert>
#include <bitset>

#include <Entity.hpp>

namespace ecs
{
	
static EntityID 				_next_id = 0;

static std::bitset<MaxEntities>	_entities;

EntityID createEntity()
{
	auto r = _next_id;
	_entities[_next_id] = true;
	while(_entities[_next_id])
		++_next_id;
	assert(_next_id < MaxEntities);
	return r;
}

void deleteEntity(EntityID id)
{
	assert(id < MaxEntities);
	_entities[id] = false;
	if(id < _next_id)
		_next_id = id;
}

}
