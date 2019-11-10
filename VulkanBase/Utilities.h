#pragma once

namespace Util
{
	static unsigned int getEntityID()
	{
		static unsigned int id = 0;
		return id++;
	}
}

struct box
{
	glm::vec3 bottomLeftNear;
	glm::vec3 topRightFar;
};

