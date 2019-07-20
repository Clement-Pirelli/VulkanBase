#pragma once

template <class T>
class ResizableArray
{
public:
	ResizableArray(){};
	ResizableArray(T* givenData, unsigned int givenCapacity, unsigned int givenDataCount) : data(givenData), capacity(givenCapacity), dataCount(givenDataCount)
	{}

	~ResizableArray()
	{
		if (data == nullptr) return;
		delete[] data;
	}

	inline T getElement(unsigned int index) const { return data[index]; }

	void push(T element)
	{
		dataCount++;
		if (dataCount > capacity)
		{
			autoResize();
		}

		data[dataCount - 1] = element;
	}

	void push(T *chunk, unsigned int count)
	{
		int oldDataCount = dataCount;
		dataCount += count;
		while(dataCount > capacity)
		{
			autoResize();
		}

		for(int i = oldDataCount; i < dataCount; i++)
		{
			data[i] = chunk[i - oldDataCount];
		}
	}

	inline T* getData() const
	{
		return data;
	}

	inline unsigned int getDataCount() const
	{
		return dataCount;
	}

	inline unsigned int getCapacity() const 
	{
		return capacity;
	}

	void resize(unsigned int newSize)
	{
		if (newSize < capacity) return;
		T* oldData = data;

		data = new T[newSize];
		if (oldData != nullptr) 
		{
			for (int i = 0; i < dataCount; i++)
			{
				data[i] = oldData[i];
			}
		}

		capacity = newSize;
	}

	inline void manualChangeDataCount(unsigned int newDataCount)
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
};