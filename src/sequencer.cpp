#include <cstring>
#include <jack/jack.h>

#include "sequencer.hpp"
#include "sequence.hpp"
#include "config.hpp"
#include "jack.hpp"

Sequencer::Sequencer(Jack jack, const char* osc_in_port, const char* osc_target_url)
{

    // Engine

    elapsed_samples = 0;
    sample_rate = jack_get_sample_rate(jack.jack_client);
    jack.set_callback(jack_callback, this);

    // Transport

    playing = false;
    cursor = 0;
    set_bpm(Config::DEFAULT_BPM);

    // Osc

    osc_port = osc_in_port;
    osc_target = lo_address_new_from_url(osc_target_url);

    osc_init();

}

Sequencer::~Sequencer()
{

    lo_address_free(osc_target);

}

void Sequencer::set_period(double p)
{

    period = p * sample_rate;

}

void Sequencer::set_bpm(float b)
{

    if (b > Config::MAX_BPM) {
        bpm = Config::MAX_BPM;
    } else if (b < Config::MIN_BPM) {
        bpm = Config::MIN_BPM;
    } else {
        bpm = b;
    }

    set_period(60. / bpm / Config::PPQN);

}

int Sequencer::jack_callback (jack_nframes_t nframes, void *arg)
{

    Sequencer *sequencer = (Sequencer *) arg;

	jack_nframes_t i;

    if (!sequencer->playing) return 0;

	for(i=0; i < nframes; i++)
	{
        sequencer->elapsed_samples += 1;
        if (sequencer->elapsed_samples >= sequencer->period) {
            sequencer->elapsed_samples = sequencer->elapsed_samples - sequencer->period;
            sequencer->play_current();
        }
	}

	return 0;

}

void Sequencer::play_current()
{

    for (auto& item: sequences) {
        item.second.play(cursor);
    }

    cursor += 1;

}


void Sequencer::play() {

    if (playing) {
        trig();
    } else {
        playing = true;
    }

}

void Sequencer::pause() {

    playing = false;

}

void Sequencer::stop() {

    pause();
    cursor = 0;
    elapsed_samples = 0;

}

void Sequencer::trig() {

    cursor = 0;
    elapsed_samples = 0;

    if (!playing) {
        play();
    }

}

void Sequencer::sequence_add(const char* address, const char* type,
                    std::map<int, double> values, int length, bool enabled, bool is_note)
{

    sequences[address] = Sequence(this, address, type, values, length, enabled, is_note);

}

void Sequencer::sequence_remove(const char* address)
{

    std::map<const char*, Sequence>::iterator it = sequences.find(address);
    sequences.erase(it);

}

void Sequencer::sequence_enable(const char* address)
{

    if (sequences.find(address) != sequences.end()) {
        sequences[address].enable();
    }

}

void Sequencer::sequence_disable(const char* address)
{

    if (sequences.find(address) != sequences.end()) {
        sequences[address].disable();
    }

}

void Sequencer::sequence_toggle(const char* address)
{

    if (sequences.find(address) != sequences.end()) {
        sequences[address].toggle();
    }

}


void osc_error(int num, const char *m, const char *path)
{
    fprintf(stderr, "liblo server error %d in path %s: %s\n", num, path, m);
}

void Sequencer::osc_init()
{

    int proto = sscanf(osc_port, "%*u%*c") == -1 ? LO_UDP : LO_UNIX;

    osc_server = lo_server_thread_new_with_proto(osc_port, proto, osc_error);

    if (!osc_server) {
        exit(0);
    }

    lo_server_thread_add_method(osc_server, "/play", NULL, Sequencer::osc_play_handler, this);
    lo_server_thread_add_method(osc_server, "/pause", NULL, Sequencer::osc_pause_handler, this);
    lo_server_thread_add_method(osc_server, "/stop", NULL, Sequencer::osc_stop_handler, this);
    lo_server_thread_add_method(osc_server, "/trig", NULL, Sequencer::osc_trig_handler, this);


    lo_server_thread_start(osc_server);
}


void Sequencer::osc_send(const char* address, const char* type, double value)
{

    if (strcmp(type, "i") == 0) {
        int ivalue = value;
        lo_send(osc_target, address, type, ivalue);
    } else {
        lo_send(osc_target, address, type, value);
    }

}

int Sequencer::osc_play_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    Sequencer *sequencer = (Sequencer *) user_data;
    sequencer->play();
	return 0;
}

int Sequencer::osc_pause_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    Sequencer *sequencer = (Sequencer *) user_data;
    sequencer->pause();
	return 0;
}

int Sequencer::osc_stop_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    Sequencer *sequencer = (Sequencer *) user_data;
    sequencer->stop();
	return 0;
}

int Sequencer::osc_trig_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    Sequencer *sequencer = (Sequencer *) user_data;
    sequencer->trig();
	return 0;
}
