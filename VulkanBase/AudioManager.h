#pragma once
#include <vector>
#include <unordered_map>
#include <al.h>
#include <alc.h>
#include "glm.hpp"

struct audioSource{ ALuint handle; };
struct audioBuffer{ ALuint handle; };

class AudioManager
{
public:

	AudioManager();

	~AudioManager();

	void setListenerTransform(glm::vec3 position, glm::vec3 direction, glm::vec3 up);

	void setVolume(ALfloat givenVolume);

	audioSource generateSource(ALfloat pitch, ALfloat gain, glm::vec3 position, glm::vec3 direction, ALboolean looping = false);

	audioBuffer generateBuffer(const char *path);

	void setSourceBuffer(audioSource source, audioBuffer buffer);

	void playSource(audioSource source);

	bool isSourcePlaying(audioSource source);

	void playOneShot(const char *path, audioSource source, float volume = 1.0f);

	void playOneShot(const char *path, glm::vec3 position, float volume = 1.0f);

private:
	std::vector<audioSource> sources;
	static const size_t oneShotSourcesCount = 12;
	audioSource oneShotSources[oneShotSourcesCount];
	size_t lastSource = oneShotSourcesCount;
	std::unordered_map<std::string, audioBuffer> buffers;
	ALCdevice *audioDevice = nullptr;
	ALCcontext *context = nullptr;
	ALfloat volume = 1.0f;
	void resetErrorStack()
	{
		alGetError();
	}
};