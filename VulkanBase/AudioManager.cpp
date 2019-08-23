#include "AudioManager.h"
#include <iostream>
#include "wavreader.h"

static inline ALenum to_al_format(unsigned int channels, unsigned int samples)
{
	bool stereo = (channels > 1);

	switch (samples) {
	case 16:
		if (stereo)
			return AL_FORMAT_STEREO16;
		else
			return AL_FORMAT_MONO16;
	case 8:
		if (stereo)
			return AL_FORMAT_STEREO8;
		else
			return AL_FORMAT_MONO8;
	default:
		return -1;
	}
}

void alCheckError()
{
#ifndef NDEBUG
	ALCenum error = alGetError();
	if (error != AL_NO_ERROR)
	{
		std::cerr << "OPENAL ERROR! ERROR CODE : " << error << std::endl;
	}
#endif
}

AudioManager::AudioManager()
{
	audioDevice = alcOpenDevice("");
#ifndef NDEBUG
	if (!audioDevice) std::cerr << "Couldn't open audio device!";
#endif
	context = alcCreateContext(audioDevice, NULL);
#ifndef NDEBUG
	if (!alcMakeContextCurrent(context)) std::cerr << "Couldn't create OpenAL context!";
#else
	alcMakeContextCurrent(context);
#endif

	for(size_t i = 0; i < oneShotSourcesCount; i++)
	{
		oneShotSources[i] = generateSource(1.0f, 1.0f, { .0,.0,.0 }, { .0,.0,.0 });
	}
	
	alCheckError();
}

AudioManager::~AudioManager()
{
	//a bit hacky - if audioSource is ever bigger than a single unsigned int, change this
	alDeleteSources((ALsizei)sources.size(), reinterpret_cast<ALuint*>(sources.data()));
	for(std::pair<std::string, audioBuffer> b : buffers)
	{
		alDeleteBuffers((ALsizei)1, &b.second.handle);
	
	}
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(audioDevice);
}

void AudioManager::setListenerTransform(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
	alListener3f(AL_POSITION, position.x, position.y, position.z);
	alCheckError();
	alListener3f(AL_VELOCITY, direction.x, direction.y, direction.z);
	alCheckError();
	ALfloat orientation[] = { position.x, position.y, position.z, up.x, up.y, up.z };
	alListenerfv(AL_ORIENTATION, orientation);
	alCheckError();
}

void AudioManager::setVolume(ALfloat givenVolume)
{
	volume = givenVolume;
	alListenerf(AL_GAIN, volume);
}

audioSource AudioManager::generateSource(ALfloat pitch, ALfloat gain, glm::vec3 position, glm::vec3 direction, ALboolean looping)
{
	audioSource returnSource;
	alGenSources((ALuint)1, &returnSource.handle);
	// check for errors

	alSourcef(returnSource.handle, AL_PITCH, pitch);
	alCheckError();
	alSourcef(returnSource.handle, AL_GAIN, gain);
	alCheckError();
	alSource3f(returnSource.handle, AL_POSITION, position.x, position.y, position.z);
	alCheckError();
	alSource3f(returnSource.handle, AL_VELOCITY, direction.x, direction.y, direction.z);
	alCheckError();
	alSourcei(returnSource.handle, AL_LOOPING, looping);
	alCheckError();
	sources.push_back(returnSource);
	return returnSource;
}

audioBuffer AudioManager::generateBuffer(const char * path)
{
	std::string pathStr = std::string(path);
	if(buffers.count(pathStr) > 0)
	{
		return buffers[pathStr];
	}
	audioBuffer buffer;
	alGenBuffers((ALuint)1, &buffer.handle);
	alCheckError();
	WavReader reader(path);
	alCheckError();
	alBufferData(buffer.handle, to_al_format(reader.getChannels(), reader.getBitsPerSample()), reader.getData(), reader.getSize(), reader.getFrequency());
	alCheckError();
	buffers.emplace(std::pair<std::string, audioBuffer>(pathStr, buffer));
	return buffer;
}

void AudioManager::setSourceBuffer(audioSource source, audioBuffer buffer)
{
	alSourcei(source.handle, AL_BUFFER, buffer.handle);
	alCheckError();
}

void AudioManager::playSource(audioSource source)
{
	alSourcePlay(source.handle);
}

bool AudioManager::isSourcePlaying(audioSource source)
{
	ALint sourceState;
	alGetSourcei(source.handle, AL_SOURCE_STATE, &sourceState);
	alCheckError();
	return (sourceState == AL_PLAYING);
}

void AudioManager::playOneShot(const char * path, audioSource source, float volume)
{
	audioBuffer b = generateBuffer(path);
	alSourcef(source.handle, AL_GAIN, volume);
	setSourceBuffer(source, b);
	playSource(source);
}

void AudioManager::playOneShot(const char * path, glm::vec3 position, float volume)
{
	lastSource++;
	if (lastSource > oneShotSourcesCount) lastSource = 0;
	audioSource s = oneShotSources[lastSource];
	if(isSourcePlaying(s))
	{
		alSourceStop(s.handle);
		alCheckError();
	}

	alSource3f(s.handle, AL_POSITION, position.x, position.y, position.z);
	alCheckError();
	playOneShot(path, s, volume);
}