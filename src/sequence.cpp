#include <lo/lo.h>

#include "sequence.hpp"
#include "osc.hpp"


Sequence::Sequence(){} // empty default constructor for std::map

Sequence::Sequence(Osc *osc_server, const char* osc_address, const char* osc_type, std::map<int, double> v, bool state, bool is_note)
{

    osc = osc_server;

    address = osc_address;
    type = osc_type;

    enabled = state;
    values = v;
    length = 192;

    note = is_note;
    note_on = false;

}

Sequence::~Sequence()
{

    disable();

}

void Sequence::enable()
{

    enabled = true;

}

void Sequence::disable()
{

    if (enabled && note_on) {
        note_on = false;
        osc->send(address, type, 0);
    }

    enabled = false;

}

void Sequence::toggle()
{

    if (enabled) {
        disable();
    } else {
        enable();
    }

}

void Sequence::play(int c)
{

    int cursor = c % length;

    if (values.find(cursor) != values.end()) {

        osc->send(address, type, values[cursor]);

        if (note) {
            note_on = values[cursor] > 0;
        }

    }

}
