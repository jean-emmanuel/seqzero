#pragma once

#include <jack/jack.h>

class Jack
{
    public:

        Jack();
        ~Jack();

        const char *client_name;
        jack_client_t *jack_client;

        void connect();
        void disconnect();
        void set_callback(JackProcessCallback callback, void *arg);

};
