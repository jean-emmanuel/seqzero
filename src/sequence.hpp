#pragma once

#include <map>

class Sequencer;

class Sequence
{
    public:

        Sequence(); // empty default constructor for std::map

        Sequence(Sequencer *seq, std::string osc_address, const char* osc_type, std::map<int, double> values, int seq_length, bool enabled, bool is_note);
        ~Sequence();

        Sequencer *sequencer;

        std::string address;
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
        void note_off();

};
