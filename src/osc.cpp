#include <cstring>
#include <cstdio>

#include "osc.hpp"

const char* int_type = "i";

Osc::Osc(const char* str_url)
{

    url = lo_address_new_from_url(str_url);

}

Osc::~Osc()
{

    lo_address_free(url);

}

void Osc::send(const char* address, const char* type, double value)
{

    if (strcmp(type, "i") == 0) {
        int ivalue = value;
        lo_send(url, address, type, ivalue);
    } else {
        lo_send(url, address, type, value);
    }

}
