/* Copyright (C) 2018 Maxim Atanasov - All Rights Reserved
 * You may not use, distribute or modify this code.
 * This code is proprietary and belongs to the "slavicTales"
 * project. See LICENCE.txt in the root directory of the project.
 *
 * E-mail: atanasovmaksim1@gmail.com
 */

#ifndef MSG_BUS_DEF
#define MSG_BUS_DEF

#include <cstdlib>
#include <iostream>
#include <util/atomic_queue/concurrentqueue.h>
#include <message_bus/subscriber.hpp>
#include <message_bus/message.hpp>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <memory>
#include <vector>
#include <cstring>
#include <message_bus/message_allocator.hpp>

///The centeral messaging system of the engine. All subsystem make extensive use of it.
/**
 *
 * Handles all passing of messages to subscribers.
 */
class message_bus{
    private:
        std::unordered_map<int, std::vector<subscriber*>> subscribers; //each message enum maps to a list of subscribers for that message
    public:
        void send_msg(message* msg);
        void subscribe(msg_type msg, subscriber* sub);
};

/**
 * Creates a new message object given the type of the message and data.
 * To create a message use "make_msg" along with "make_data<>" - you do not need to manage the memory of the data only the lifetime of a message.
 * Any data you get out of a message (using get_data()) is guaranteed to be available until you
 * call "destroy_msg()" on that message, afterwards the results are undefined.
 * Shared pointers are used internally to manage memory.
 * @param name The type of message. See <b>ST::msg_type</b>.
 * @param data The data the message carries - created with <b>make_data<>()</b> or is <b>nullptr</b>
 * @return A new message object.
 */
inline message* make_msg(msg_type name, const std::shared_ptr<void>& data){
    return msg_memory.allocate_message(name, data);
}

/**
 * Destroys a message. Call this when you absolutely no longer need the message.
 * @param msg The message to destroy.
 */
inline void destroy_msg(message* msg){
    msg_memory.deallocate(msg->get_id());
}

/**
 * Makes a copy of itself.
 * @return A new message that is an exact copy of the original.
 */
inline message* message::make_copy(){
    return msg_memory.allocate_message(this->msg_name, this->data);
}

/**
 * Really just a wrapper around the cumbersome - static_cast<std::shared_ptr<void>>(std::make_shared<T>(data));
 * @tparam T The type of the data.
 * @param data The data itself - usually passed by value.
 * @return A new std::shared_ptr<void> representing the data.
 */
template <class T> std::shared_ptr<void> make_data(T data){
    return std::make_shared<T>(data);
}

#endif
