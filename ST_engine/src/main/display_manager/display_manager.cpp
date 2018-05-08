/* Copyright (C) 2018 Maxim Atanasov - All Rights Reserved
 * You may not use, distribute or modify this code.
 * This code is proprietary and belongs to the "slavicTales"
 * project. See LICENCE.txt in the root directory of the project.
 *
 * E-mail: atanasovmaksim1@gmail.com
 */

#include <display_manager/display_manager.hpp>
#include <console/log.hpp>

/**
 * Closes the Display Manager.
 * Destroys the window and quits SDL.
 */
void display_manager::close(){
    SDL_FreeSurface(icon);
    SDL_DestroyWindow(window);
    window = nullptr;
    SDL_Quit();
}

/**
 * Initializes the window and SDL.
 * @param msg_bus a pointer to the global message bus
 * @param tsk_mngr a pointer to the global task manager
 * @return This function either returns 0 or exits the program with exit code 1.
 */
int display_manager::initialize(message_bus* msg_bus, task_manager* tsk_mngr){
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    if(IMG_Init(IMG_INIT_PNG) < 0){
        fprintf(stderr, "Failed to initialize SDL_IMG: %s\n", IMG_GetError());
        exit(1);
    }
    gTask_manager = tsk_mngr;
    gMessage_bus = msg_bus;
	SDL_GetDisplayMode(0, 0, &DM);
	width = DM.w;
    height = DM.h;
    window = SDL_CreateWindow
    (
        "SlavicTales", 0,
        0,
		width,
        height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );
    log(INFO, "Current screen resolution is " + std::to_string(width) + "x" + std::to_string(height));
	gMessage_bus->send_msg(make_msg(REAL_SCREEN_COORDINATES, make_data<std::tuple<int,int>>(std::make_tuple(width, height))));

    //Load and set icon
    icon = IMG_Load("levels/icon.png");
    SDL_SetWindowIcon(window, icon);

    gMessage_bus->subscribe(SET_FULLSCREEN, &msg_sub);
    gMessage_bus->subscribe(SET_WINDOW_BRIGHTNESS, &msg_sub);
    return 0;
}

/**
 * Performs the update for the display_manager on a task thread.
 * @param arg pointer to an display_manager (a <b>this</b> pointer basically) as the
 * function must be static.
 */
void display_manager::update_task(void* mngr){
    auto self = (display_manager*)mngr;
    self->handle_messages();
}

/**
 * Retrieves messages from the subscriber object and
 * performs the appropriate actions.
 */
void display_manager::handle_messages(){
    message* temp = msg_sub.get_next_message();
    while(temp != nullptr){
        if(temp->msg_name == SET_FULLSCREEN){
            auto arg = static_cast<bool*>(temp->get_data());
            set_fullscreen(*arg);
            gMessage_bus->send_msg(make_msg(FULLSCREEN_STATUS, make_data<bool>(*arg)));
        }
        else if(temp->msg_name == SET_WINDOW_BRIGHTNESS){
            auto arg = static_cast<float*>(temp->get_data());
            set_brightness(*arg);
            log(SUCCESS, "Brightness set to: " + std::to_string(*arg));
        }
        destroy_msg(temp);
        temp = msg_sub.get_next_message();
    }
}

/**
 *
 * @return Returns a handle to the SDL_Window*
 */
SDL_Window* display_manager::get_window(){
    return window;
}

/**
 * Sets the window to fullscreen or windowed mode
 * @param arg True for fullscreen or false for windowed.
 */
void display_manager::set_fullscreen(bool arg){
    if(arg) {
        SDL_SetWindowFullscreen(window, 1);
    }else{
        SDL_SetWindowFullscreen(window, 0);
    }
}

/**
 * This function should set the brightness.
 * However, it does not function.
 * @param arg a float indicating the value.
 */
void display_manager::set_brightness(float arg) {
    SDL_SetWindowBrightness(this->window, arg);
}