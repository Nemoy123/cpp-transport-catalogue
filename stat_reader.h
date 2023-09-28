#pragma once


namespace transcat::output {

std::ostream& DateOutput () {
    std::ostream& out = std::cout;
    return out;
}

}