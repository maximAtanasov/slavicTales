/* This file is part of the "slavicTales" project.
 * You may use, distribute or modify this code under the terms
 * of the GNU General Public License version 2.
 * See LICENCE.txt in the root directory of the project.
 *
 * Author: Maxim Atanasov
 * E-mail: atanasovmaksim1@gmail.com
 */

#ifndef FONT_CACHE_DEF
#define FONT_CACHE_DEF

#include <defs.hpp>
#include <list>
#include <tuple>

#ifndef CACHEHASH_DEF
#define CACHEHASH_DEF

typedef std::tuple<std::string, std::string, int> font_cache_tuple;
namespace std{

    ///defines a hash operator for the font_cache_tuple type - this is needed because we use it in a ska::bytell_hash_map
    template <> struct hash<font_cache_tuple>{
          std::size_t operator()(const font_cache_tuple& k) const{
          using std::size_t;
          using std::hash;
          using std::string;
          return ((hash<string>()(std::get<0>(k)) ^ (hash<string>()(std::get<1>(k)) << 1)) >> 1) ^ (hash<int>()(std::get<2>(k)) << 1);
        }
    };
}

//redefine the equals operator for two font caches tuples <string, string, int>
bool operator==(const std::tuple<std::string, std::string, int>& tpl1, const std::tuple<std::string, std::string, int>& tpl2);

//A few typedefs to make working with these types easier.
typedef std::pair<font_cache_tuple, SDL_Texture*> key_pair;
typedef std::list<key_pair> cache_list;
typedef ska::bytell_hash_map<font_cache_tuple, cache_list::iterator> cache_hash;

#endif

///A LRU Cache that caches rendered strings, it is used in the drawTextNormal() method of the renderer class
/**
 * Caches textures and the string, font+size used to render them.
 */
class font_cache{
    private:
        uint32_t entries = 0;
        cache_list cache;
        cache_hash hash;
        uint32_t cache_size = 0;
        void move_to_front(std::list<key_pair>& list,std::list<key_pair>::iterator element);
    public:
        font_cache() = default;
        void set_max(uint32_t max);
        ~font_cache();
        void cache_string(std::string str, SDL_Texture* texture, std::string font, uint8_t size);
        SDL_Texture* get_cached_string(std::string str, std::string font, uint8_t size);
        void clear();
};

#endif // FONT_CACHE_DEF
