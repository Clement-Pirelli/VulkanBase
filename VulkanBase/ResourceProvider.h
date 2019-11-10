#ifndef RESOURCE_PROVIDER_H_DEFINED
#define RESOURCE_PROVIDER_H_DEFINED

#include <unordered_map>
#include <string>

template<typename T>
class ResourceProvider
{
public:

	static T &getResource(std::string name)
	{
		if (cache.count(name) > 0)
		{
			return cache[name];
		}

		cache.insert(std::make_pair<std::string, T>(std::string(name), T::create(name)));
		return cache[name];
	}

	static void clearResources()
	{
		for (auto p : cache)
		{
			T::destroy(p.second);
		}
		cache.clear();
	}

private:
	static std::unordered_map<std::string, T> cache;
	ResourceProvider() = delete;
};

template<typename T>
std::unordered_map<std::string, T> ResourceProvider<T>::cache;

#endif