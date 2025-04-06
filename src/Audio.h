#pragma once

#include "Module.h"
#include "SDL2/SDL_mixer.h"
#include <list>

#define DEFAULT_MUSIC_FADE_TIME 2.0f

struct _Mix_Music;

class Audio : public Module
{
public:

	Audio();

	// Destructor
	virtual ~Audio();

	// Called before render is available
	bool Awake();

	// Called before quitting
	bool CleanUp();

	// Play a music file
	bool PlayMusic(const char* path, float fadeTime = DEFAULT_MUSIC_FADE_TIME, int trackId);
    int LoadMusic(const char* path);
	bool StopMusic();
    bool musicVolume(int v);

	// Load a WAV in memory
	int LoadFx(const char* path);
	bool PlayFx(int fx, int repeat = 0, int channel = -1);
   	int randomFx(int min, int max);
	bool StopFxChannel(int channel);//lowers fx volume until it finishes
	bool VolumeFx(int id,int v);//lowers fx volume until it finishes
	
	


	
	

private:
	Mix_Music* music;
	std::list<Mix_Chunk*> fx;
	std::list<_Mix_Music*> tracks;
};
