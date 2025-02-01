#include "env.hpp"

Env::Env() {}

Env::~Env() {}

const Env &Env::get_instance() {
    static Env instance;
    return instance;
}
