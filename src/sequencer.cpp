#include <cstdio>
#include <jack/jack.h>

#include "sequencer.hpp"
#include "config.hpp"
#include "jack.hpp"

int jack_callback (jack_nframes_t nframes, void *arg)
{

    Sequencer *sequencer = (Sequencer *) arg;

	int i;

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

Sequencer::Sequencer(Jack jack)
{

    cursor = 0;
    elapsed_samples = 0;

    sample_rate = jack_get_sample_rate(jack.jack_client);

    set_bpm(Config::DEFAULT_BPM);

    jack.set_callback(jack_callback, this);
    // fprintf (stderr,"%i",sample_rate);


}

Sequencer::~Sequencer()
{

}

void Sequencer::set_period(float p)
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

    // fprintf (stderr,"%i", cursor);
    cursor += 1;
    // loop through sequences...

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

void Sequencer::sequence_add(std::string address, std::map<int, float> values)
{

    sequences[address] = Sequence(values);

}
