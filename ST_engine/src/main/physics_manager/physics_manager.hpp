/* Copyright (C) 2018 Maxim Atanasov - All Rights Reserved
 * You may not use, distribute or modify this code.
 * This code is proprietary and belongs to the "slavicTales"
 * project. See LICENCE.txt in the root directory of the project.
 *
 * E-mail: atanasovmaksim1@gmail.com
 */

#ifndef PHYSICS_DEF
#define PHYSICS_DEF

#include <defs.hpp>
#include <game_manager/level/level.hpp>
#include <game_manager/level/entity.hpp>
#include <task_manager/task_manager.hpp>

///This class handles all physics related actions in the engine.
/**
 * Messages this subsystem listens to: <br>
 *
 * <b>SET_GRAVITY</b> - Sets the current gravity.
 *
 * Message must contain: a pointer to a <b>int8_t</b>.
 *
 *  <b>SET_FRICTION</b> - Sets the current friction.
 *
 * Message must contain: a pointer to a <b>int8_t</b>.
 *
 *  <b>SET_FLOOR</b> - Sets the floor of the level.
 *
 * Message must contain: a pointer to a <b>int32_t</b>.
 *
 *  <b>PAUSE_PHYSICS</b> - Pauses all physics.
 *
 * Message must contain: a <b>nullptr</b>.
 *
 * <b>UNPAUSE_PHYSICS</b> - Resumes all physics.
 *
 * Message must contain: a <b>nullptr</b>.
 */
class physics_manager{
    private:
        message_bus* gMessage_bus{};
		task_manager* gTask_manager{};
        subscriber msg_sub{};
        int8_t gravity = 0;
        int8_t friction = 0;
        int32_t level_floor = 0;
		bool physics_paused = false;
        std::vector<ST::entity>* entities{};
        
        int check_collision(uint64_t, std::vector<ST::entity>* entities);
        int entity_set_x(int x, uint64_t, std::vector<ST::entity>* entities);
        int entity_set_y(int y, uint64_t, std::vector<ST::entity>* entities);

		void process_horizontal();
		void process_vertical();
        void handle_messages();

    public:
        physics_manager() = default;
        int initialize(message_bus* msg_bus, task_manager* tsk_mngr);
        void update(ST::level_data* data);
        void close();
};

//INLINED METHODS

/**
 * Responds to messages from the subscriber object and updates the physics if they are not paused.
 * @param data A pointer to the level data. (containing the entities that we need).
 */
inline void physics_manager::update(ST::level_data* data){
	handle_messages();
	if(physics_paused){
		return;
	}
	entities = &data->entities;
	process_horizontal();
	process_vertical();
}

#endif //PHYSICS_DEF