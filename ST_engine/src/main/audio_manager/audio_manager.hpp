/* This file is part of the "slavicTales" project.
 * You may use, distribute or modify this code under the terms
 * of the GNU General Public License version 2.
 * See LICENCE.txt in the root directory of the project.
 *
 * Author: Maxim Atanasov
 * E-mail: atanasovmaksim1@gmail.com
 */

#ifndef AUDIO_MNGR_DEF
#define AUDIO_MNGR_DEF

#include <SDL.h>
#include <SDL_mixer.h>
#include <assets_manager/assets.hpp>
#include <message_bus/message_bus.hpp>
#include <task_manager/task_manager.hpp>
#include <console/log.hpp>
#include <task_manager/task_allocator.hpp>

///This object is responsible for playing sounds and music
/**
 *
 * Messages this subsystem listens to: <br>
 *
 * <b>PLAY_SOUND</b> - Plays a sound.
 *
 * Message must contain: a pointer to a <b>std::tuple<size_t, int, int></b> containing the chunk to play,
 * the volume to play it at, and the amount of times to loop it. <br>
 *
 * <b>PLAY_MUSIC</b> - Plays music.
 *
 * Message must contain: a pointer to a <b>std::tuple<size_t, int, int></b> containing the music to play,
 * the volume to play it at, and the amount of times to loop it. <br>
 *
 * <b>STOP_MUSIC</b> - Stops the music.
 *
 * Message must contain: a <b>nullptr</b>.<br>
 *
 * <b>STOP_ALL_SOUNDS</b> - Stops all playing sounds.
 *
 * Message must contain: a <b>nullptr</b>.<br>
 *
 * <b>TOGGLE_AUDIO</b> - Toggles the audio on or off.
 *
 * Message must contain: a <b>nullptr</b>.<br>
 *
 * <b>ASSETS</b> - Updates the internal pointer to the assets. This messages is recieved from the asset manager
 * whenever new assets are loaded. <br>
 *
 * Message must contain: a pointer to a <b>ST::assets*</b> containing the assets struct. <br>
 *
 * <b>SET_VOLUME</b> - Sets the current playback volume. <br>
 *
 * Message must contain: a pointer to a <b>uint8_t</b>. <br>
 *
 */
class audio_manager{
    friend class audio_manager_test;
    private:

        int volume = MIX_MAX_VOLUME;

        ///subscriber object to receive messages
        subscriber msg_sub{};

        ///External dependencies
        ST::assets* assets_ptr{}; //-Delivered as a message
        message_bus* gMessage_bus{}; //-Delivered in constructor
        task_manager* gTask_manager{}; //-Delivered in constructor

        ///internal update functions
        void handle_messages();


        ///Audio control functions
        void play_music(size_t arg, uint8_t volume, int8_t loops) const;
        void play_sound(size_t arg, uint8_t volume, int8_t loops) const;
        void mute();
        void unmute();
        void stop_music();
        void stop_channels();
        void set_volume(uint8_t arg);
        static void update_task(void* arg);

    public:
        audio_manager(message_bus* msg_bus, task_manager* tsk_mngr);
        ~audio_manager();
        void update();
};

//INLINED METHODS

/**
 * Starts the update_task() method using the task manager.
 */
inline void audio_manager::update(){
    gTask_manager->start_task_lockfree(make_task(update_task, this, nullptr, -1));
}


/**
 * Sets the current playback volume.
 * @param arg the volume level - valid from 0 to 128.
 */
inline void audio_manager::set_volume(uint8_t arg) {
    volume = arg;
    unmute();
}

/**
 * Mutes all audio channels.
 */
inline void audio_manager::mute(){
    Mix_Volume(-1, 0);
    Mix_VolumeMusic(0);
}

/**
 * Unmutes all audio channels.
 */
inline void audio_manager::unmute(){
    Mix_Volume(-1, volume);
    Mix_VolumeMusic(volume);
}

/**
 * Plays a sound.
 * @param arg The sound to play - a hashed value of the original filename.
 * @param volume The volume to play the chunk at.
 * @param loops How many times to play it.
 */
inline void audio_manager::play_sound(size_t arg, uint8_t volume, int8_t loops) const{
    Mix_Chunk* data = assets_ptr->chunks[arg];
    if(data != nullptr){
        Mix_VolumeChunk(data, volume);
        if(Mix_PlayChannel( -1, data, loops ) == -1){
            log(ERROR, "Mix_PlayChannel Error " + std::string(Mix_GetError()));
        }
    }
}

/**
 *
 * @param arg The music to play - a hashed value of the original filename.
 * @param volume The volume to play the music at.
 * @param loops How many times to play it, -1 will loop indefinitely.
 */
inline void audio_manager::play_music(size_t arg, uint8_t volume, int8_t loops) const{
    Mix_Music* data = assets_ptr->music[arg];
    if(data != nullptr){
        Mix_VolumeMusic(volume);
        if(Mix_PlayMusic(data, loops) == -1){
            log(ERROR, "Mix_PlayMusic Error " + std::string(Mix_GetError()));
        }
        Mix_VolumeMusic(this->volume);
    }
}

/**
 * Stop the music that is currently playing.
 */
inline void audio_manager::stop_music(){
    Mix_PauseMusic();
}

/**
 * Stop all sound channels (except for music)
 */
inline void audio_manager::stop_channels(){
    Mix_HaltChannel(-1);
}

#endif
