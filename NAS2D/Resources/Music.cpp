// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2019 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// =
// = Acknowledgement of your use of NAS2D is appriciated but is not required.
// ==================================================================================
#include "Music.h"
#include "MusicInfo.h"

#include "../Filesystem.h"
#include "../Utility.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <iostream>
#include <string>

using namespace NAS2D;

std::map<std::string, MusicInfo> MUSIC_REF_MAP; /**< Lookup table for music resource references. */

void updateMusicReferenceCount(const std::string& name);

/**
 * C'tor.
 *
 * \param filePath	Path of the music file to load.
 */
Music::Music(const std::string& filePath) : Resource(filePath)
{
	load();
}


/**
 * Copy c'tor.
 */
Music::Music(const Music& rhs) : Resource(rhs.mResourceName)
{
	auto it = MUSIC_REF_MAP.find(mResourceName);
	if (it != MUSIC_REF_MAP.end())
	{
		it->second.refCount++;
	}

	mIsLoaded = rhs.mIsLoaded;
}


/**
 * Copy operator.
 */
Music& Music::operator=(const Music& rhs)
{
	if (this == &rhs) { return *this; }

	updateMusicReferenceCount(mResourceName);

	mResourceName = rhs.mResourceName;

	auto it = MUSIC_REF_MAP.find(mResourceName);
	if (it != MUSIC_REF_MAP.end())
	{
		it->second.refCount++;
		mIsLoaded = rhs.mIsLoaded;
	}
	else
	{
		mIsLoaded = false;
	}

	return *this;
}


/**
 * D'tor.
 */
Music::~Music()
{
	updateMusicReferenceCount(mResourceName);
}


/**
 * Loads a specified music file.
 *
 * \note	This function is called internally during instantiation.
 */
void Music::load()
{
	if (MUSIC_REF_MAP.find(mResourceName) != MUSIC_REF_MAP.end())
	{
		MUSIC_REF_MAP.find(mResourceName)->second.refCount++;
		mIsLoaded = true;
		return;
	}

	File* file = new File(Utility<Filesystem>::get().open(mResourceName));
	if (file->empty())
	{
		delete file;
		return;
	}

	Mix_Music* music = Mix_LoadMUS_RW(SDL_RWFromConstMem(file->raw_bytes(), static_cast<int>(file->size())), 0);
	if (!music)
	{
		std::cout << "Music::load(): " << Mix_GetError() << std::endl;
		return;
	}

	auto& record = MUSIC_REF_MAP[mResourceName];
	record.buffer = file;
	record.music = music;
	record.refCount++;

	mIsLoaded = true;
}


// ==================================================================================
// = Unexposed module-level functions defined here that don't need to be part of the
// = API interface.
// ==================================================================================

/**
* Internal function used to clean up references to fonts when the Music
* destructor or copy assignment operators are called.
*
* \param	name	Name of the Music to check against.
*/
void updateMusicReferenceCount(const std::string& name)
{
	auto it = MUSIC_REF_MAP.find(name);
	if (it == MUSIC_REF_MAP.end())
	{
		return;
	}

	--it->second.refCount;

	// No more references to this resource.
	if (it->second.refCount < 1)
	{
		if (it->second.music)
		{
			Mix_FreeMusic(static_cast<Mix_Music*>(it->second.music));
		}

		if (it->second.buffer)
		{
			delete static_cast<File*>(it->second.buffer);
		}

		MUSIC_REF_MAP.erase(it);
	}
}
