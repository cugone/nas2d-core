// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2020 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// =
// = Acknowledgement of your use of NAS2D is appriciated but is not required.
// ==================================================================================
#pragma once

#include <string>


struct Mix_Chunk;


namespace NAS2D {

/**
 *  Sound resource.
 *
 *  Represents a Sound.
 */
class Sound
{
public:
	explicit Sound(const std::string& filePath);

	Sound(const Sound& other) = delete;
	Sound(Sound&& other) = delete;
	Sound& operator=(const Sound& rhs) = delete;
	Sound& operator=(Sound&& other) = delete;

	~Sound();

	const std::string& name() const { return mResourceName; }

protected:
	friend class MixerSDL;
	Mix_Chunk* sound() const;

private:
	std::string mResourceName; /**< File path */
	Mix_Chunk* mMixChunk{nullptr};
};

} // namespace
