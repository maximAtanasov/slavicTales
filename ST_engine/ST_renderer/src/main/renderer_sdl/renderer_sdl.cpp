/* This file is part of the "ST" project.
 * You may use, distribute or modify this code under the terms
 * of the GNU General Public License version 2.
 * See LICENCE.txt in the root directory of the project.
 *
 * Author: Maxim Atanasov
 * E-mail: maxim.atanasov@protonmail.com
 *
 */

#include "font_cache.hpp"
#include <renderer_sdl.hpp>

namespace ST::renderer_sdl {
        void cache_font(TTF_Font *Font, uint16_t font_and_size);
    }

static SDL_Renderer *sdl_renderer;

//reference to a window
static SDL_Window *window;

//height and width of the renderer
static int16_t width;
static int16_t height;

//Textures with no corresponding surface in our assets need to be freed
static ska::bytell_hash_map<uint16_t, SDL_Texture *> textures{};

static ska::bytell_hash_map<uint16_t, SDL_Surface *> *surfaces_pointer;
static ska::bytell_hash_map<uint16_t, TTF_Font *> *fonts_pointer;


//the fonts in this table do not need to be cleaned - these are just pointer to Fonts stored in the asset_manager and
//that will handle the cleanup
static ska::bytell_hash_map<uint16_t, TTF_Font *> fonts{};

//we do however need to cleanup the cache as that lives on the GPU
static ska::bytell_hash_map<uint16_t, std::vector<SDL_Texture *>> fonts_cache{};

static bool vsync = false;

static bool singleton_initialized = false;

/**
 * Initializes the renderer.
 * @param window The window to bind this renderer to.
 * @param width The virtual width of the window.
 * @param height The virtual height of the window.
 * @return Always 0.
 */
int8_t ST::renderer_sdl::initialize(SDL_Window* r_window, int16_t r_width, int16_t r_height){
    font_cache::set_max(100);

    if(singleton_initialized){
        throw std::runtime_error("The renderer cannot be initialized more than once!");
    }else{
        singleton_initialized = true;
    }

    //initialize renderer
	window = r_window;
	width = r_width;
	height = r_height;
    if(vsync){
        sdl_renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }else{
        sdl_renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
    }
    SDL_RenderSetLogicalSize(sdl_renderer, width, height);
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ); //Linear texture filtering
    SDL_SetHint( SDL_HINT_RENDER_BATCHING, "1" );
    set_draw_color(0, 0, 0, 255);
    return 0;
}

/**
 * Closes the renderer. Deletes all textures and fonts.
 */
void ST::renderer_sdl::close(){
    for (auto& it : fonts) {
        if (it.second != nullptr) {
            if (fonts[it.first] != nullptr) {
                for(auto& k : fonts_cache[it.first]) {
                    SDL_DestroyTexture(k);
                }
                fonts[it.first] = nullptr;
            }
            it.second = nullptr;
        }
    }
    for ( auto& it : textures){
        if(it.second != nullptr ){
            SDL_DestroyTexture(textures[it.first]);
            it.second = nullptr;
        }
    }
    font_cache::clear();
    font_cache::close();
    SDL_DestroyRenderer(sdl_renderer);
    sdl_renderer = nullptr;
    singleton_initialized = false;
}

/**
 * Text rendering method for non-ASCII text -- works with cyrillic, expanded Latin (Spanish, German, etc..) and I guess all of UTF8
 * @param arg The font to render with.
 * @param arg2 The text to render.
 * @param x The x position to render at.
 * @param y The y position to render at.
 * @param color_font The color to render with.
 * @param size The size of the font to render at.
 * @return The width of the rendered string in pixels
 *
 * Note that the font must previously be loaded at the selected size.
 */
uint16_t ST::renderer_sdl::draw_text_lru_cached(uint16_t font, const std::string& arg2, int x, int y, SDL_Color color_font){
    TTF_Font* _font = fonts[font];
    int32_t texW = 0;
    if(_font != nullptr) [[likely]] {
        int32_t texH;
        SDL_Texture* cached_texture = font_cache::get_cached_string(arg2, font);
        if(cached_texture != nullptr){ //if the given string (with same size and font) is already cached, get it from cache
            SDL_QueryTexture(cached_texture, nullptr, nullptr, &texW, &texH);
            SDL_Rect Rect = {x, y - texH, texW, texH};
            SDL_SetTextureColorMod(cached_texture, color_font.r, color_font.g, color_font.b);
            SDL_RenderCopy(sdl_renderer, cached_texture, nullptr, &Rect);
        }else{ //else create a texture, render it, and then cache it - this is costly, so pick a good cache size
            SDL_Surface* text = TTF_RenderUTF8_Blended(_font, arg2.c_str(), color_font);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, text);
            SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
            SDL_Rect Rect = {x, y - texH, texW, texH};
            SDL_SetTextureColorMod(texture, color_font.r, color_font.g, color_font.b);
            SDL_RenderCopy(sdl_renderer, texture, nullptr, &Rect);
            SDL_FreeSurface(text);
            font_cache::cache_string(arg2, texture, font);
        }
    }
    return static_cast<uint16_t>(texW);
}

/**
 * This will draw text using cached glyphs - the fastest way possible, works only with ASCII and relatively simple fonts
 * (any complex cursive fonts won't be rendered properly).
 * @param arg The font to render with.
 * @param arg2 The text to render.
 * @param x The x position to render at.
 * @param y The y position to render at.
 * @param color_font The color to render with.
 * @param size The size of the font to render at.
 * @return The width of the rendered string in pixels
 *
 * Note that the font must previously be loaded at the selected size.
 */
uint16_t ST::renderer_sdl::draw_text_cached_glyphs(uint16_t font, const std::string& arg2, const int x, const int y, const SDL_Color color_font) {
    int32_t tempX = 0;
    auto cached_vector = fonts_cache.find(font);
    if(cached_vector != fonts_cache.end()) [[likely]] {
        std::vector<SDL_Texture*> tempVector = cached_vector->second;
        if(!tempVector.empty()) [[likely]] {
            int32_t texW, texH;
            tempX = x;
            const char* arg3 = arg2.c_str();
            for(int j = 0; arg3[j] != 0; j++){
                SDL_Texture* texture = tempVector.at(static_cast<unsigned int>(arg3[j]-32));
                SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
                SDL_Rect Rect = {tempX, y - texH, texW, texH};
                SDL_SetTextureColorMod(texture, color_font.r, color_font.g, color_font.b);
                SDL_RenderCopy(sdl_renderer, texture, nullptr, &Rect);
                tempX += texW;
            }
        }
    }
    return static_cast<uint16_t>(tempX - x);
}

/**
 * Upload all surface to the GPU. (Create textures from them).
 * @param surfaces The surfaces to upload.
 */
void ST::renderer_sdl::upload_surfaces(ska::bytell_hash_map<uint16_t, SDL_Surface*>* surfaces){
	if(surfaces != nullptr){
		surfaces_pointer = surfaces;
        for ( auto& it : *surfaces){
            if(it.second == nullptr && textures[it.first] != nullptr){
                SDL_DestroyTexture(textures[it.first]);
                textures[it.first] = nullptr;
            }
            else if(it.second != nullptr){
                    if(textures[it.first] != nullptr){
                    SDL_DestroyTexture(textures[it.first]);
                    textures[it.first] = nullptr;
                }
                textures[it.first] = SDL_CreateTextureFromSurface(sdl_renderer, it.second);
            }
        }
    }
}

/**
 * Upload fonts to the GPU. (save and cache their glyphs).
 */
void ST::renderer_sdl::upload_fonts(ska::bytell_hash_map<uint16_t, TTF_Font*>* fonts_t){
    if(fonts_t != nullptr){
		fonts_pointer = fonts_t;
        for ( auto& it : *fonts_t){
            if(it.second == nullptr){
                for(auto& k : fonts_cache[it.first]){
                    SDL_DestroyTexture(k);
                }
                fonts[it.first] = nullptr;
            }
            else if(it.second != nullptr){
                if(fonts[it.first] != nullptr){
                    for(auto& k : fonts_cache[it.first]){
                        SDL_DestroyTexture(k);
                    }
                    fonts[it.first] = nullptr;
                }
                fonts[it.first] = it.second;
                cache_font(fonts[it.first], it.first);
            }
        }
    }
}

/**
 * Caches all glyphs of a font at a given size.
 * Works with the draw_text_cached method.
 * Do not confuse this method with the font_cache class, they have nothing in common, this caches fonts to be used
 * with ASCII strings, font_cache is a LRU cache that works with any string.
 * @param Font The Font to render with.
 * @param font_and_size The name+size of the font.
 */
void ST::renderer_sdl::cache_font(TTF_Font* Font, uint16_t font_and_size){
    SDL_Color color_font = {255, 255, 255, 255};
    char temp[2];
    temp[1] = 0;
    std::vector<SDL_Texture*> tempVector;
    for(char j = 32; j < 127; j++) {
        temp[0] = j;
        SDL_Surface* glyph = TTF_RenderUTF8_Blended(Font, temp, color_font);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, glyph);
        tempVector.push_back(texture);
        SDL_FreeSurface(glyph);
    }
    fonts_cache[font_and_size] = tempVector;
}

/**
 * Turns on vsync.
 */
void ST::renderer_sdl::vsync_on(){
    SDL_RendererInfo info;
    SDL_GetRendererInfo(sdl_renderer, &info);
    if(!(info.flags & SDL_RENDERER_PRESENTVSYNC)){ // NOLINT(hicpp-signed-bitwise)
        vsync = true;
        close();
        initialize(window, width, height);
        upload_surfaces(surfaces_pointer);
        upload_fonts(fonts_pointer);
    }
}

/**
 * Turns off vsync.
 */
void ST::renderer_sdl::vsync_off(){
    SDL_RendererInfo info;
    SDL_GetRendererInfo(sdl_renderer, &info);
    if(info.flags & SDL_RENDERER_PRESENTVSYNC){ // NOLINT(hicpp-signed-bitwise)
        vsync = false;
        close();
        initialize(window, width, height);
        upload_surfaces(surfaces_pointer);
        upload_fonts(fonts_pointer);
    }
}

//INLINED METHODS

/**
 * Draw a texture at a given position.
 * @param arg The hash of the texture name.
 * @param x The X position to render at.
 * @param y The Y position to render at.
 */
void ST::renderer_sdl::draw_texture(const uint16_t arg, int32_t x, int32_t y) {
    auto data = textures.find(arg);
    auto texture = reinterpret_cast<SDL_Texture*>((data != textures.end())*reinterpret_cast<uint64_t>(data->second));
    int tex_w, tex_h;
    SDL_QueryTexture(texture, nullptr, nullptr, &tex_w, &tex_h);
    SDL_Rect src_rect = {x, y - tex_h, tex_w, tex_h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &src_rect);
}

/**
 * Draw a texture at a given position and scale it.
 * @param arg The hash of the texture name.
 * @param x The X position to render at.
 * @param y The Y position to render at.
 */
void ST::renderer_sdl::draw_texture_scaled(const uint16_t arg, int32_t x, int32_t y, float scale_x, float scale_y) {
    auto data = textures.find(arg);
    auto texture = reinterpret_cast<SDL_Texture*>((data != textures.end())*reinterpret_cast<uint64_t>(data->second));
    int tex_w, tex_h;
    SDL_QueryTexture(texture, nullptr, nullptr, &tex_w, &tex_h);
    SDL_Rect dst_rect = {x,
                         y - static_cast<int>(static_cast<float>(tex_h) * scale_y),
                         static_cast<int>(static_cast<float>(tex_w) * scale_x),
                         static_cast<int>(static_cast<float>(tex_h) * scale_y)};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst_rect);
}

/**
 * Draws a filled rectangle on the screen.
 * @param x The X position to draw at.
 * @param y The Y position to draw at.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @param color The color of the rectangle.
 */
void ST::renderer_sdl::draw_rectangle_filled(int32_t x, int32_t y, int32_t w, int32_t h, SDL_Color color) {
    SDL_Rect Rect = {x, y, w, h};
    SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(sdl_renderer, &Rect);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
}

/**
 * Draws a rectangle on the screen.
 * @param x The X position to draw at.
 * @param y The Y position to draw at.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @param color The color of the rectangle.
 */
void ST::renderer_sdl::draw_rectangle(int32_t x, int32_t y, int32_t w, int32_t h, SDL_Color color) {
    SDL_Rect Rect = {x, y, w, h};
    SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(sdl_renderer, &Rect);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
}

/**
 * Draws a texture that fills the entire screen (Background).
 * @param arg The hash of the texture name.
 */
void ST::renderer_sdl::draw_background(const uint16_t arg) {
    auto data = textures.find(arg);
    auto texture = reinterpret_cast<SDL_Texture*>((data != textures.end())*reinterpret_cast<uint64_t>(data->second));
    SDL_RenderCopy(sdl_renderer, texture, nullptr, nullptr);
}

/**
 * @param arg The hash of the name of the texture.
 * @param offset The offset in the texture for the parallax effect
 */
void ST::renderer_sdl::draw_background_parallax(const uint16_t arg, const uint16_t offset) {
    auto data = textures.find(arg);
    auto texture = reinterpret_cast<SDL_Texture*>((data != textures.end())*reinterpret_cast<uint64_t>(data->second));

    int tex_w, tex_h;
    SDL_QueryTexture(texture, nullptr, nullptr, &tex_w, &tex_h);
    float bg_ratio = (float)tex_w/(float)width;
    int src_offset = (int)((float)offset*bg_ratio);

    SDL_Rect dst_rect1 = {0, 0, width - offset, height};
    SDL_Rect src_rect1 = {src_offset, 0, tex_w - src_offset, tex_h};
    SDL_Rect src_rect2 = {0, 0, src_offset, tex_h};
    SDL_Rect dst_rect2 = {width - offset, 0, offset, height};

    SDL_RenderCopy(sdl_renderer, texture, &src_rect1, &dst_rect1);
    SDL_RenderCopy(sdl_renderer, texture, &src_rect2, &dst_rect2);
}

/**
 * Draws a texture that is a spritesheet.
 * @param arg The hash of the name of the spritesheet.
 * @param x The X position to render at.
 * @param y The Y position to render at.
 * @param sprite The number of the sprite in the texture. (Column in the spritesheet).
 * @param animation The number of the animation in the texture (Row in the spritesheet).
 * @param animation_num The total number of animations in a spritesheet (Rows in the spritesheet).
 * @param sprite_num The total number of sprites in a spritesheet. (Columns in a spritesheet).
 */
void ST::renderer_sdl::draw_sprite(uint16_t arg, int32_t x, int32_t y, uint8_t sprite, uint8_t animation, uint8_t animation_num, uint8_t sprite_num) {
    auto data = textures.find(arg);
    auto texture = reinterpret_cast<SDL_Texture*>((data != textures.end())*reinterpret_cast<uint64_t>(data->second));

    int tex_w, tex_h;
    SDL_QueryTexture(texture, nullptr, nullptr, &tex_w, &tex_h);
    int temp1 = tex_h / animation_num;
    int temp2 = tex_w / sprite_num;
    SDL_Rect dst_rect = {x, y - temp1, temp2, temp1};
    SDL_Rect src_rect = {sprite * temp2, temp1 * (animation - 1), temp2, temp1};
    SDL_RenderCopy(sdl_renderer, texture, &src_rect, &dst_rect);
}

/**
 * Draws a texture that is a spritesheet.
 * @param arg The hash of the name of the spritesheet.
 * @param x The X position to render at.
 * @param y The Y position to render at.
 * @param sprite The number of the sprite in the texture. (Column in the spritesheet).
 * @param animation The number of the animation in the texture (Row in the spritesheet).
 * @param animation_num The total number of animations in a spritesheet (Rows in the spritesheet).
 * @param sprite_num The total number of sprites in a spritesheet. (Columns in a spritesheet).
 */
void ST::renderer_sdl::draw_sprite_scaled(uint16_t arg, int32_t x, int32_t y, uint8_t sprite, uint8_t animation, uint8_t animation_num, uint8_t sprite_num, float scale_x, float scale_y) {
    auto data = textures.find(arg);
    auto texture = reinterpret_cast<SDL_Texture*>((data != textures.end())*reinterpret_cast<uint64_t>(data->second));

    int tex_w, tex_h;
    SDL_QueryTexture(texture, nullptr, nullptr, &tex_w, &tex_h);
    int temp1 = tex_h / animation_num;
    int temp2 = tex_w / sprite_num;

    SDL_Rect dst_rect = {x,
                         y - static_cast<int>(static_cast<float>(temp1) * scale_y),
                         static_cast<int>(static_cast<float>(temp2) * scale_x),
                         static_cast<int>(static_cast<float>(temp1) * scale_y)};
    SDL_Rect src_rect = {sprite * temp2, temp1 * (animation - 1), temp2, temp1};
    SDL_RenderCopy(sdl_renderer, texture, &src_rect, &dst_rect);
}

/**
 * Draws an animated overlay.
 * Works similarly to draw_sprite, except only one animation is supported.
 * @param arg The hash of the texture name.
 * @param sprite The number of the sprite to use.
 * @param sprite_num The total number of frames this spritesheet has.
 */
void ST::renderer_sdl::draw_overlay(uint16_t arg, uint8_t sprite, uint8_t sprite_num) {
    auto data = textures.find(arg);
    auto texture = reinterpret_cast<SDL_Texture*>((data != textures.end())*reinterpret_cast<uint64_t>(data->second));

    int32_t tex_w, tex_h;
    SDL_QueryTexture(texture, nullptr, nullptr, &tex_w, &tex_h);
    SDL_Rect src_rect = {sprite * (tex_w / sprite_num), 0, tex_w / sprite_num, tex_h};
    SDL_RenderCopy(sdl_renderer, texture, &src_rect, nullptr);
}

/**
 * Clears the screen given a color
 */
void ST::renderer_sdl::clear_screen(SDL_Color color) {
    SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(sdl_renderer);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 0);
}

/**
 * Clears the screen with the color set by set_draw_color
 */
void ST::renderer_sdl::clear_screen() {
    SDL_RenderClear(sdl_renderer);
}

/**
 * Sets a draw color.
 * @param r Red value.
 * @param g Green value.
 * @param b Blue value.
 * @param a Alpha value.
 */
void ST::renderer_sdl::set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetRenderDrawColor(sdl_renderer, r, g, b, a);
}

/**
 * Presents the framebuffer to the window.
 */
void ST::renderer_sdl::present() {
    SDL_RenderPresent(sdl_renderer);
}

/**
 * Sets the internal rendering resolution
 * @param r_width Width
 * @param r_height Height
 */
void ST::renderer_sdl::set_resolution(int16_t r_width, int16_t r_height) {
    width = r_width;
    height = r_height;
    SDL_RenderSetLogicalSize(sdl_renderer, width, height);
}
