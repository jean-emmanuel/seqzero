#include "osc.hpp"

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

    // if (type == "i") {
    //     // fprintf (stderr,"i");
    //     int ivalue = value;
    //     lo_send(url, address, type, ivalue);
    // } else {
        lo_send(url, address, type, value);
    // }


}
