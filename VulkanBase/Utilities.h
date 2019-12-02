#pragma once

namespace Util
{
	static unsigned int getEntityID()
	{
		static unsigned int id = 0;
		return id++;
	}

	static unsigned long x = 123456789, y = 362436069, z = 521288629;

	//from : https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
	static unsigned long rand() {
		unsigned long t;
		x ^= x << 16;
		x ^= x >> 5;
		x ^= x << 1;

		t = x;
		x = y;
		y = z;
		z = t ^ x ^ y;

		return z;
	}

}

struct box
{
	glm::vec3 bottomLeftNear;
	glm::vec3 topRightFar;
};

