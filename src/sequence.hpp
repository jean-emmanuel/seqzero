#pragma once

#include <map>

#include "osc.hpp"

class Sequence
{
    public:

        Sequence(); // empty default constructor for std::map

        Sequence(Osc *osc_server, const char* osc_address, const char* osc_type, std::map<int, double> values, bool enabled, bool is_note);
        ~Sequence();

        Osc *osc;

        const char* address;
        const char* type;

        bool enabled;

        int length;
        std::map<int, double> values;

        bool note;
        bool note_on;

        void enable();
        void disable();
        void toggle();

        void play(int cursor);

};
