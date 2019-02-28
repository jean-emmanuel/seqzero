#pragma once

#include <map>

class Sequencer;

typedef std::map<int, double> ValueMap;
typedef ValueMap::iterator ValueMapIterator;

class Sequence
{
    public:

        Sequence(); // empty default constructor for std::map

        Sequence(Sequencer *seq, std::string osc_address, const char* osc_type, ValueMap values, int seq_length, bool enabled, bool is_note);
        ~Sequence();

        Sequencer *sequencer;

        std::string address;
        const char* type;

        bool enabled = false;

        int length;
        ValueMap values;

        bool note;
        bool note_on;

        void enable();
        void disable();
        void toggle();

        void play(int cursor);
        void note_off();

        void feed_status(bool deleted);

};
