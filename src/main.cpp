#include <arena/arena.hpp>
#include <iostream>

int main(int, char**) {
    auto c = arena::make_component();
    std::cout << "comp.x = " << c.x <<std::endl;
    auto lang = "C++" ;
    std::cout << "Hello and welcome to " << lang << "!\n";

    for (int i = 1; i <= 5; i++) {
        std::cout << "i = " << i << std::endl;
    }

    std::getchar();
    return 0;
}
