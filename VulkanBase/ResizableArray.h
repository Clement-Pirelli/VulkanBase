#pragma once

template <class T>
class ResizableArray
{
public:
	ResizableArray(){};
	ResizableArray(T* givenData, unsigned int givenCount) : data(givenData), count(givenCount)
	{
		for (int i = 0; i < givenCount; i++)
		{
			if (givenData[i] == nullptr)
			{
				dataCount = i; return;
			}
		}
	}

	~ResizableArray()
	{
		if (data == nullptr) return;
		delete[] data;
	}

	T getElement(unsigned int index){ return data[index]; }
	T getElement(int index){ return data[index]; }

	void push(T element)
	{
		dataCount++;
		if (dataCount > count)
		{
			resize();
		}

		data[dataCount - 1] = element;
	}

private:
	T* data = nullptr;
	unsigned int count = 0;
	unsigned int dataCount = 0;

	void resize()
	{
		if (count == 0)
		{
			count = 1;
			data = new T[count];
			return;
		}

		T* old = data;
		data = new T[count * 2];

		for (int i = 0; i < dataCount; i++)
		{
			data[i] = old[i];
		}
		count *= 2;
	}

};