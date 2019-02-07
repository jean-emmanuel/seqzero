#include <stdlib.h>
#include <cstdio>

#include "jack.hpp"
#include "config.hpp"

Jack::Jack()
{

    client_name = "SeqZero";

}

Jack::~Jack()
{

}

void Jack::connect()
{

    jack_status_t status;
    jack_client = jack_client_open (client_name, JackNullOption, &status, NULL);

    if (jack_client == NULL) {
        fprintf (stderr, "jack_client_open() failed, "
             "status = 0x%2.0x\n", status);
        if (status & JackServerFailed) {
            fprintf (stderr, "Unable to connect to JACK server\n");
        }
        exit(1);
    }

    if (status & JackServerStarted) {
        fprintf (stderr, "JACK server started\n");
    }

    if (status & JackNameNotUnique) {
        client_name = jack_get_client_name(jack_client);
        fprintf (stderr, "unique name `%s' assigned\n", client_name);
    }

}

void Jack::disconnect()
{
    jack_client_close(jack_client);
}

void Jack::set_callback(JackProcessCallback callback, void *arg)
{

    jack_set_process_callback(jack_client, callback, arg);

    if (jack_activate(jack_client)) {
        fprintf (stderr, "cannot activate client");
        exit(1);
    }

}
