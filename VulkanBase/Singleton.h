#pragma once

template<class T>
class Singleton
{
public:

	static T* getInstance()
	{
		return instance;
	}

	static void setInstance(T *givenInstance)
	{
		instance = givenInstance;
	}

	static void removeInstance()
	{
		instance = nullptr;
	}
private:

	static T *instance;
};


template <class T> T *Singleton<T>::instance = nullptr;