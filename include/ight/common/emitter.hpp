/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_COMMON_EMITTER_HPP
# define IGHT_COMMON_EMITTER_HPP

/*
 * \brief Templates to implement event emitters.
 *
 * The following is an example of usage. You must provide the
 * `using` statements inside the class to disambiguate.
 *
 *     using namespace ight::common::emitter;
 *
 *     class Antani : public EmitterVoid,
 *             public Emitter<int, std::string> {
 *     public:
 *         using EmitterVoid::on;
 *         using EmitterVoid::emit;
 *         using Emitter<int, std::string>::on;
 *         using Emitter<int, std::string>::emit;
 *     };
 */

#include <functional>
#include <map>
#include <vector>

namespace ight {
namespace common {
namespace emitter {

class EmitterVoid {
    std::map<std::string, std::vector<std::function<void()>>> events;
public:
    void on(const std::string name, std::function<void()> func) {
        events[name].push_back(func);
    }
    void emit(const std::string name) {
        auto it = events.find(name);
        if (it != events.end()) {
            for (auto func : it->second) {
                func();
            }
        }
    }
};

template<typename... T> class Emitter {
    std::map<std::string, std::vector<std::function<void(T...)>>> events;
public:
    void on(const std::string name, std::function<void(T...)> func) {
        events[name].push_back(func);
    }
    void emit(const std::string name, T... arg) {
        auto it = events.find(name);
        if (it != events.end()) {
            for (auto func : it->second) {
                func(arg...);
            }
        }
    }
};

}}}
#endif
