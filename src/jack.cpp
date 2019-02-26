#include <stdlib.h>
#include <cstdio>

#include "jack.hpp"
#include "config.hpp"
#include "sequencer.hpp"

Jack::Jack(Sequencer *seq, const char* name, bool _transport)
{

    sequencer = seq;
    client_name = name;
    transport_state = JackTransportStopped;

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

        if (jack_position.valid & JackPositionBBT) {
            if (jack_position.beats_per_minute > 0 && jack_position.beats_per_minute != sequencer->bpm) {
                sequencer->set_bpm(jack_position.beats_per_minute);
            }
        }

        if (state != transport_state) {

            switch (state) {

                case JackTransportStopped:
                    // printf( "[JackTransportStopped]\n" );
                    sequencer->pause(true);
                    break;

                case JackTransportStarting:
                    // printf( "[JackTransportStarting]\n" );
                    sequencer->pause(true);
                    break;

                case JackTransportRolling:
                    // printf( "[JackTransportRolling]\n" );
                    sequencer->set_cursor((long)
                        jack_frame *
                        Config::PPQN *
                        sequencer->bpm / (jack_position.frame_rate * 60.0)
                    , true);
                    sequencer->play(true);
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

void Jack::set_cursor(long c)
{

    jack_position_t jack_position;
    jack_transport_query(jack_client, &jack_position);

    if (!(jack_position.valid & JackPositionBBT)) {
        jack_position.ticks_per_beat = 1920;
        jack_position.beats_per_minute = sequencer->bpm;
    }

    jack_nframes_t frame = c /
                    (jack_position.ticks_per_beat * jack_position.beats_per_minute / (jack_position.frame_rate * 60.0));

    jack_transport_locate(jack_client, frame);

}

void Jack::play()
{

    jack_transport_start(jack_client);

}

void Jack::pause()
{

    jack_transport_stop(jack_client);

}
