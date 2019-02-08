#include "sequence.hpp"
#include "sequencer.hpp"

Sequence::Sequence(){} // empty default constructor for std::map

Sequence::Sequence(Sequencer *seq, std::string osc_address, const char* osc_type, std::map<int, double> v, int seq_length, bool state, bool is_note)
{

    sequencer = seq;

    address = osc_address;
    type = osc_type;

    enabled = state;
    values = v;
    length = seq_length;

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
        sequencer->osc_send(address, type, 0);
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

    if (!enabled) return;

    int cursor = c % length;

    if (values.find(cursor) != values.end()) {

        sequencer->osc_send(address, type, values[cursor]);

        if (note) {
            note_on = values[cursor] > 0;
        }

    }

}
