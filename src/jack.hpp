#pragma once

#include <jack/jack.h>

class Sequencer;

class Jack
{
    public:

        Jack(Sequencer *seq, const char* name, bool transport);
        ~Jack();

        Sequencer *sequencer;

        const char *client_name;
        jack_client_t *jack_client;

        void connect();
        void disconnect();

        jack_time_t get_time();

        // transport

        bool transport;
        jack_transport_state_t transport_state;

};
