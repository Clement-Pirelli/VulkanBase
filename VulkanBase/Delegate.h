#pragma once

union delegateInfo
{
	struct
	{
		int ix, iy, iz;
	};

	struct
	{
		float fx, fy, fz;
	};
};

//base code from : https://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
class Delegate
{
public:

	Delegate() : object(0), function(0){}

	//creates the delegate
	template <class T, void (T::*TMethod)(delegateInfo&)>
	static Delegate makeDelegate(T *givenObject)
	{
		Delegate returnDelegate;
		returnDelegate.object = givenObject;
		returnDelegate.function = method_stub<T, TMethod>;
		return returnDelegate;
	}

	void operator()(delegateInfo&info)
	{
		(*function)(object, info);
	}

private:
	//function which takes a void pointer and event information
	typedef void(*intermediaryFunction)(void*object, delegateInfo&);

	template <class T, void (T::*TMethod)(delegateInfo&)>
	static void method_stub(void* givenObject, delegateInfo&givenInfo)
	{
		T* p = static_cast<T*>(givenObject);
		(p->*TMethod)(givenInfo); // #2
	}

	//anonymous object which we cast to its actual type in the function
	void* object;
	intermediaryFunction function;
};