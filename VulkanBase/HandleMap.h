#ifndef HANDLEMAP_H_DEFINED
#define HANDLEMAP_H_DEFINED

#include <stdint.h>
#include <unordered_map>

template<typename T>
class HandleMap
{
public:
	uint32_t insert(T givenT)
	{
		uint32_t insertedHandle = lastHandle;
		data[insertedHandle] = givenT;
		lastHandle++;
		return insertedHandle;
	}

	void remove(uint32_t givenKey)
	{
		data.erase(givenKey);
	}

	T &operator[](uint32_t givenHandle)
	{
		return data[givenHandle];
	}

	void clear()
	{
		lastHandle = 0;
		data.empty();
	}

	std::unordered_map<uint32_t, T> &getRawMap(){ return data; }

private:
	uint32_t lastHandle = 0;
	std::unordered_map<uint32_t, T> data;
};

#endif
