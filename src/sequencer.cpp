#include <cstring>
#include <jack/jack.h>

#include "sequencer.hpp"
#include "sequence.hpp"
#include "config.hpp"
#include "jack.hpp"

int jack_callback (jack_nframes_t nframes, void *arg)
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

Sequencer::Sequencer(Jack jack, const char* str_url)
{

    playing = false;

    cursor = 0;
    elapsed_samples = 0;

    url = lo_address_new_from_url(str_url);

    sample_rate = jack_get_sample_rate(jack.jack_client);

    set_bpm(Config::DEFAULT_BPM);

    jack.set_callback(jack_callback, this);
    // fprintf (stderr,"%i",sample_rate);

}

Sequencer::~Sequencer()
{

    lo_address_free(url);

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

void Sequencer::stop() {

    playing = false;

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

void Sequencer::send(const char* address, const char* type, double value)
{

    if (strcmp(type, "i") == 0) {
        int ivalue = value;
        lo_send(url, address, type, ivalue);
    } else {
        lo_send(url, address, type, value);
    }

}
