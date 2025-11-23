#pragma once

namespace arena {

    struct component {
        int x;
        int y;
        int z;
    }; // component

    inline component make_component() {
        return component{};
    }

} // arena
