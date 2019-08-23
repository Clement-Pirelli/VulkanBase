#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "wavreader.h"

#pragma pack(push, 1)
struct RiffWaveHeader
{
	unsigned int chunkID		= 0;
	unsigned int chunkSize		= 0;
	unsigned int format			= 0;
	unsigned int subchunk1ID	= 0;
	unsigned int subchunk1Size	= 0;
	short  audioFormat			= 0;
	short  numChannels			= 0;
	unsigned int sampleRate		= 0;
	unsigned int byteRate		= 0;
	short  blockAlign			= 0;
	short  bitsPerSample		= 0;
	unsigned int subchunk2ID	= 0;
	unsigned int subchunk2Size	= 0;
};
#pragma pack(pop)

WavReader::WavReader(const char *path)
{
#ifndef NDEBUG
	wavReaderError error = openFile(path);
	if (error != NO_ERROR) std::cerr << "Couldn't open file at path : " << path << " Error code is : " << error << std::endl;
#else
	openFile(path);
#endif
}

WavReader::~WavReader()
{
	closeFile();
}

wavReaderError WavReader::openFile(const char *path)
{
	
	FILE *file;
	fopen_s(&file, path, "rb");
	if (!file) return FILE_NOT_FOUND;

	uint8_t header[4];
	if (fread(header, 1, sizeof(header), file) != sizeof(header)) {
		fclose(file);
		return FILE_UNSUPPORTED;
	}

	fseek(file, 0, SEEK_SET);
	if (memcmp(header, "RIFF", 4) == 0) {
		int valid = 1;
		RiffWaveHeader header;
		size_t read = fread(&header, 1, sizeof(RiffWaveHeader), file);
		if (read != sizeof(RiffWaveHeader))   valid = 0;
		if (header.format != 0x45564157)      valid = 0; // 'WAVE'
		if (header.subchunk1ID != 0x20746D66) valid = 0; // 'fmt '
		if (header.subchunk2ID != 0x61746164) valid = 0; // 'data'
		if (header.audioFormat != 1)          valid = 0; // PCM
		if (!valid) {
			fclose(file);
			return FILE_UNSUPPORTED;
		}

		size = header.subchunk2Size;
		bitsPerSample = header.bitsPerSample;
		channels = header.numChannels;
		frequency = header.sampleRate;
		
		data = new uint8_t[header.subchunk2Size];
		fread(data, 1, header.subchunk2Size, file);
		return NO_ERROR;
	}

	fclose(file);
	return FILE_UNSUPPORTED;
}

void WavReader::closeFile()
{
	fclose((FILE*)data);
	delete[] data;
	data = nullptr;
}