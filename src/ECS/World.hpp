#pragma once

#include <array>

/// Will replace Scene (?)
namespace ecs
{

constexpr size_t MaxEntities = 65536;
using EntityID = uint64_t;

template<typename T>
std::array<T, MaxEntities>	components;

EntityID createEntity();

template<typename T>
T& assign(EntityID id, T&& component)
{
	components<T>[id] = component;
}

};
