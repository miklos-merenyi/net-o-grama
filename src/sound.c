#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "debug.h"

// audio vars
Uint32 audio_len;
Uint8 *audio_pos;

struct sound
{
    char* name;
    Mix_Chunk *audio_chunk;
    struct sound* next;
};
struct sound* soundCache = NULL;

/***********************************************************
synopsis: walk the module level soundCache until the required
      name is found.  when found, return the audio data
      if name is not found, return NULL instead.

inputs:   name - the unique id string of the required sound

outputs:  returns a chunk of audio or NULL if not found
***********************************************************/
Mix_Chunk* getSound(char* name)
{

    struct sound* currentSound = soundCache;

    while (currentSound!=NULL)
    {

        if(!strcmp(currentSound->name, name))
        {
            return currentSound->audio_chunk;
            break;
        }
        currentSound = currentSound->next;
    }

    return NULL;
}


/***********************************************************
synopsis: push a sound onto the soundCache

inputs:   soundCache - pointer to the head of the soundCache
      name - unique id string for the sound, this is used
             to later play the sound
      filename - the filename of the WAV file

outputs:  n/a
***********************************************************/
void pushSound(struct sound** soundCache, char* name, char* filename)
{

    struct sound* thisSound = NULL;

    thisSound = malloc(sizeof(struct sound));
    thisSound->name = malloc(sizeof(name)*strlen(name));
    strcpy(thisSound->name, name);
    thisSound->next = *soundCache;

    // Attempt to load a sample
    thisSound->audio_chunk = Mix_LoadWAV(filename);

    *soundCache = thisSound;
}


/***********************************************************
synopsis: push all the game sounds onto the soundCache
      linked list.  Not that soundCache is passed into
      pushSound by reference, so that the head pointer
      can be updated

inputs:   pointer to the soundCache

outputs:  n/a
***********************************************************/
void bufferSounds(struct sound** soundCache)
{

    pushSound(&(*soundCache),"click-answer", "audio/click-answer.wav");
    pushSound(&(*soundCache),"click-shuffle", "audio/click-shuffle.wav");
    pushSound(&(*soundCache),"foundbig", "audio/foundbig.wav");
    pushSound(&(*soundCache),"found", "audio/found.wav");
    pushSound(&(*soundCache),"clear", "audio/clearword.wav");
    pushSound(&(*soundCache),"duplicate", "audio/duplicate.wav");
    pushSound(&(*soundCache),"badword", "audio/badword.wav");
    pushSound(&(*soundCache),"shuffle", "audio/shuffle.wav");
    pushSound(&(*soundCache),"clock-tick", "audio/clock-tick.wav");
    pushSound(&(*soundCache),"found2", "audio/found2.wav");

}


/***********************************************************
synopsis: free all of the data in the audio buffer
      the audio buffer is a module variable

inputs:   n/a

outputs:  n/a
***********************************************************/
void clearSoundBuffer()
{

    struct sound* currentSound = soundCache, *previousSound = NULL;

    while (currentSound!=NULL)
    {

        Mix_FreeChunk(currentSound->audio_chunk);
        free(currentSound->name);
        previousSound = currentSound;
        currentSound = currentSound->next;
        free(previousSound);
    }
}


int initSound()
{

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        debug(0, "Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    //
    // buffer sounds
    //
    int audio_rate = MIX_DEFAULT_FREQUENCY;
    Uint16 audio_format = AUDIO_S16;
    int audio_channels = 1;
    int audio_buffers = 256;

    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
    {
        debug(0,"unable to open audio!\n");
        exit(1);
    }

    bufferSounds(&soundCache);
    return 1;

}


int endSound()
{
    Mix_CloseAudio();
    clearSoundBuffer(&soundCache);
    return 0;
}
