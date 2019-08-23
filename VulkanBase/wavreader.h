#pragma once
#ifndef WAVREADER_DEFINED
#define WAVREADER_DEFINED

enum wavReaderError {
	FILE_NOT_FOUND = -3,
	FILE_UNSUPPORTED = -2,
	INVALID = -1,
	NO_ERROR = 0
};

class WavReader
{
public:

	WavReader(const char *path);
	~WavReader();
	
	inline unsigned int getSize(){ return size; }
	inline unsigned int getBitsPerSample(){ return bitsPerSample; }
	inline unsigned int getChannels(){ return channels; }
	inline unsigned int getFrequency(){ return frequency; }
	inline void *getData(){ return data; }
private:

	unsigned int size = 0U;
	unsigned int bitsPerSample = 0U;
	unsigned int channels = 0U;
	unsigned int frequency = 0U;
	void *data = nullptr;

	wavReaderError openFile(const char *filename);
	void closeFile();
};

#endif