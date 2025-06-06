#include "Audio.h"
#include "Log.h"
#include <time.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

Audio::Audio() : Module()
{
	music = NULL;
	name = "audio";
}

// Destructor
Audio::~Audio()
{}

// Called before render is available
bool Audio::Awake()
{
	LOG("Loading Audio Mixer");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		LOG("SDL_INIT_AUDIO could not initialize! SDL_Error: %s\n", SDL_GetError());
		active = false;
		ret = true;
	}

	// Load support for the JPG and PNG image formats
	int flags = MIX_INIT_OGG;
	int init = Mix_Init(flags);

	if((init & flags) != flags)
	{
		LOG("Could not initialize Mixer lib. Mix_Init: %s", Mix_GetError());
		active = false;
		ret = true;
	}

	// Initialize SDL_mixer
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		LOG("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		active = false;
		ret = true;
	}

	//load music and sound in array
	
	return ret;
}

// Called before quitting
bool Audio::CleanUp()
{
	if(!active)
		return true;

	LOG("Freeing sound FX, closing Mixer and Audio subsystem");

	if(music != NULL)
	{
		Mix_FreeMusic(music);
	}

	for (const auto& fxItem : fx) {
		Mix_FreeChunk(fxItem);
	}
	fx.clear();

	Mix_CloseAudio();
	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	return true;
}

// Play a music file
bool Audio::PlayMusic(const char* path, float fadeTime, int trackId)
{
	bool ret = true;

	if(!active)
		return false;

	/*if (trackId > 0 && trackId <= tracks.size())
	{
		auto musIt = tracks.begin();
		std::advance(musIt, trackId - 1);
		music = *musIt;
	}*/
	if(music != NULL)
	{
		if(fadeTime > 0.0f)
		{
			Mix_FadeOutMusic(int(fadeTime * 1000.0f));
		}
		else
		{
			Mix_HaltMusic();
		}

		// this call blocks until fade out is done
		Mix_FreeMusic(music);
	}
	
	music = Mix_LoadMUS(path);
	
	if(music == NULL)
	{
		LOG("Cannot load music %s. Mix_GetError(): %s\n", path, Mix_GetError());
		ret = false;
	}
	else
	{
		if(fadeTime > 0.0f)
		{
			if(Mix_FadeInMusic(music, -1, (int) (fadeTime * 1000.0f)) < 0)
			{
				LOG("Cannot fade in music %s. Mix_GetError(): %s", path, Mix_GetError());
				ret = false;
			}
		}
		else
		{
			if(Mix_PlayMusic(music, -1) < 0)
			{
				LOG("Cannot play in music %s. Mix_GetError(): %s", path, Mix_GetError());
				ret = false;
			}
		}
	}

	LOG("Successfully playing %s", path);
	return ret;
}
int  Audio::LoadMusic(const char* path)
{
	int ret = 0;
	if (!active)
		return 0;

    _Mix_Music* track = Mix_LoadMUS(path);

	if (track == NULL)
	{
		LOG("Cannot load music wav %s. Mix_GetError(): %s", path, Mix_GetError());
	}
	else
	{
		tracks.push_back(track);
		ret = (int)tracks.size();
	}

}
// Load WAV
int Audio::LoadFx(const char* path)
{
	int ret = 0;

	if(!active)
		return 0;

	Mix_Chunk* chunk = Mix_LoadWAV(path);

	if(chunk == NULL)
	{
		LOG("Cannot load fx wav %s. Mix_GetError(): %s", path, Mix_GetError());
	}
	else
	{
		fx.push_back(chunk);
		ret = (int)fx.size();
	}

	return ret;
}

bool Audio::musicVolume(int v)
{
	bool ret = true;
	if (music!= NULL)
	{
		Mix_VolumeMusic(v);
	}
	return ret;

}
bool Audio::StopMusic()
{
	bool ret = true;
	if (music != NULL)
	{
		Mix_HaltMusic();
	}
	return ret;
}

// Play WAV
bool Audio::PlayFx(int id, int repeat,int channel)
{
	bool ret = false;

	if(!active)
		return false;

	if(id > 0 && id <= fx.size())
	{
		auto fxIt = fx.begin();
		std::advance(fxIt, id-1);
		Mix_PlayChannel(channel, *fxIt, repeat);
	}

	return ret;
}
int Audio::randomFx(int min, int max)
{
	int randomId = min + (rand() % (max - min + 1));
	return randomId;
}
bool  Audio::StopFxChannel(int channel)
{
	bool ret = true;
	
		Mix_HaltChannel(channel);
		
	return ret;
}
bool Audio::VolumeFx(int id, int v)
{
	bool ret = true;
	if (id > 0 && id <= fx.size())
	{
		auto fxIt = fx.begin();
		std::advance(fxIt, id - 1);
		Mix_VolumeChunk(*fxIt, v);
	}
	return ret;
}
