/* Copyright (C) 2018 Maxim Atanasov - All Rights Reserved
 * You may not use, distribute or modify this code.
 * This code is proprietary and belongs to the "slavicTales"
 * project. See LICENCE.txt in the root directory of the project.
 *
 * E-mail: atanasovmaksim1@gmail.com
 */

#ifndef ASST_DEF
#define ASST_DEF

#include <assets_manager/assets.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <defs.hpp>
#include <message_bus/message_bus.hpp>
#include <task_manager/task_manager.hpp>

///This object is responsible for loading/unloading assets
class assets_manager{
    friend class asset_manager_test;
private:
        message_bus* gMessage_bus{};
        task_manager* gTask_manager{};
        subscriber* msg_sub{};
        SDL_Renderer* renderer{};
        ST::assets all_assets;
        std::unordered_map<std::string, int> count;
        int load_asset(std::string path);
        int unload_asset(std::string path);
        int unload_assets_from_list(std::string path);
        int load_assets_from_list(std::string path);
        int load_assets_from_binary(const std::string& path);
        void handle_messages();
        static void update_task(void* arg);

    public:
        //assets_manager() = default;
        int initialize(message_bus* msg_bus, task_manager* tsk_mngr);
        void close();
        void update();
};

//INLINED METHODS

//will start the update task
inline void assets_manager::update(){
    gTask_manager->start_task_lockfree(new ST::task(update_task, this, nullptr, -1));
}

#endif
