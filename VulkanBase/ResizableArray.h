#pragma once

template <class T>
class ResizableArray
{
public:
	ResizableArray(){};
	ResizableArray(T* givenData, unsigned int givenCount) : data(givenData), capacity(givenCount)
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

	T getElement(unsigned int index) const { return data[index]; }

	void push(T element)
	{
		dataCount++;
		if (dataCount > capacity)
		{
			autoResize();
		}

		data[dataCount - 1] = element;
	}

	T* getData() const
	{
		return data;
	}

	unsigned int getDataCount() const
	{
		return dataCount;
	}

	unsigned int getCapacity() const 
	{
		return capacity;
	}

	void resize(unsigned int newSize)
	{
		if (newSize < capacity) return;
		T* old = data;

		data = new T[newSize];
		copyData(old, data);
		capacity = newSize;
	}

	void manualChangeDataCount(unsigned int newDataCount)
	{
		dataCount = newDataCount;
	}

private:
	T* data = nullptr;
	unsigned int capacity = 0;
	unsigned int dataCount = 0;

	void autoResize()
	{
		if(capacity > 0)
			resize(capacity*2);
		else resize(1);
	}

	void copyData(T* old, T* newData)
	{
		if (old == nullptr) return;
		for (int i = 0; i < dataCount; i++)
		{
			data[i] = old[i];
		}
	}

};