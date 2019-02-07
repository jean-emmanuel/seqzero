#pragma once

#include <map>


class Sequence
{
    public:

        Sequence(); // empty default constructor for std::map

        Sequence(std::map<int, float> values);
        ~Sequence();

};
