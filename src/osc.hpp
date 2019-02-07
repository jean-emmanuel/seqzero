#pragma once

#include <lo/lo.h>

class Osc
{

    public:

        Osc(const char* str_url);
        ~Osc();

        void send(const char* address, const char* type, double value);

        lo_address url;

};
