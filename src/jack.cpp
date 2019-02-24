#include <stdlib.h>
#include <cstdio>

#include "jack.hpp"
#include "config.hpp"
#include "sequencer.hpp"

Jack::Jack(Sequencer *seq, const char* name, bool _transport)
{

    sequencer = seq;
    client_name = name;

    transport = _transport;

    connect();

}

Jack::~Jack()
{

    disconnect();

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

jack_time_t Jack::get_time()
{

    if (transport) {

        jack_position_t jack_position;
        jack_transport_state_t state = jack_transport_query	(jack_client, &jack_position);
        jack_nframes_t jack_frame = jack_get_current_transport_frame(jack_client);

        if (state != transport_state) {

            switch (state) {

                case JackTransportStopped:
                    // printf( "[JackTransportStopped]\n" );
                    sequencer->pause();
                    break;

                case JackTransportStarting:
                    // printf( "[JackTransportStarting]\n" );

                    sequencer->cursor = (long)
                        jack_frame *
                        jack_position.ticks_per_beat *
                        jack_position.beats_per_minute / (jack_position.frame_rate * 60.0);

                    sequencer->set_bpm(jack_position.beats_per_minute);
                    sequencer->pause();

                    break;

                case JackTransportRolling:
                    // printf( "[JackTransportRolling]\n" );
                    sequencer->play();
                    break;

                case JackTransportNetStarting:
                case JackTransportLooping:
                    break;

            }

            transport_state = state;

        }


        return  1000000.0 * jack_frame / jack_position.frame_rate;

    } else {

        return jack_get_time();

    }

}
