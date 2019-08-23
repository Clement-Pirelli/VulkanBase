#pragma once
#include "Optional.h"
typedef unsigned int uint32_t;

struct QueueFamilyIndices
{
	Optional<uint32_t> graphicsFamily;
	Optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.isSet() && presentFamily.isSet();
	}
};