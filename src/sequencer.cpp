#include <cstring>
#include <jack/jack.h>
#include <json.h>

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

    for (auto& item: sequence_map) {
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

void Sequencer::sequence_add(std::string address, const char* type,
                    std::map<int, double> values, int length, bool enabled, bool is_note)
{

    sequence_map[address] = Sequence(this, address, type, values, length, enabled, is_note);

}

void Sequencer::sequence_control(std::string address, int command)
{

    switch(command) {
        case SEQ_ENABLE:
            sequence_map[address].enable();
            break;
        case SEQ_DISABLE:
            sequence_map[address].disable();
            break;
        case SEQ_TOGGLE:
            sequence_map[address].toggle();
            break;
        case SEQ_REMOVE:
            std::map<std::string, Sequence>::iterator it = sequence_map.find(address);
            sequence_map.erase(it);
            break;
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

    lo_server_thread_add_method(osc_server, "/sequence", "ss", Sequencer::osc_seqctrl_handler, this);
    lo_server_thread_add_method(osc_server, "/sequence", "sss", Sequencer::osc_seqwrite_handler, this);

    lo_server_thread_start(osc_server);

}


void Sequencer::osc_send(std::string address, const char* type, double value)
{

    if (strcmp(type, "i") == 0) {
        int ivalue = value;
        lo_send(osc_target, address.c_str(), type, ivalue);
    } else {
        lo_send(osc_target, address.c_str(), type, value);
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


int Sequencer::osc_seqctrl_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{

    Sequencer *sequencer = (Sequencer *) user_data;

    std::string address = &argv[0]->s;
    std::string command_str = &argv[1]->s;

    if (address.c_str()[0] !=  '/') return 0;

    int command = sequencer->osc_sequence_commands[command_str];

    if (!command) return 0;

    if (sequencer->sequence_map.find(address) != sequencer->sequence_map.end()) {

        sequencer->sequence_control(address, command);

    } else {

        for (auto item = sequencer->sequence_map.cbegin(); item != sequencer->sequence_map.cend();) {

            std::string item_address = item->second.address;
            item++;

            if (lo_pattern_match(item_address.c_str(), address.c_str())) {

                sequencer->sequence_control(item_address, command);

            }

        }

    }


	return 0;

}

int Sequencer::osc_seqwrite_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{

    Sequencer *sequencer = (Sequencer *) user_data;


    std::string address = &argv[0]->s;
    std::string command_str = &argv[1]->s;
    const char * json_str = &argv[2]->s;

    int command = sequencer->osc_sequence_commands[command_str];

    if (address.c_str()[0] !=  '/' || command != SEQ_WRITE) return 0;

    json_object * json = json_tokener_parse(json_str);

    if (!json) return 0;

    json_object * walker;

    bool is_note = false;
    bool enabled = false;
    int length = 0;
    const char* osc_type = "f";
    std::map<int, double> values;

    if (json_object_object_get_ex(json, "note", &walker)) {
        is_note = json_object_get_boolean(walker);
    }

    if (json_object_object_get_ex(json, "enabled", &walker)) {
        enabled = json_object_get_boolean(walker);
    }

    if (json_object_object_get_ex(json, "length", &walker)) {
        length = json_object_get_int(walker);
    }

    if (json_object_object_get_ex(json, "type", &walker)) {
        osc_type = json_object_get_string(walker);
    }

    if (json_object_object_get_ex(json, "values", &walker)) {
        json_object_object_foreach(walker, key, val) {
            int k = atoi(key);
            values[k] = json_object_get_double(val);
        }
    }


    sequencer->sequence_add(address, osc_type, values, length, enabled, is_note);

	return 0;

}

void Sequencer::osc_feed() {

    std::string json = "{";

    json += "\"bpm\":" + std::to_string(bpm) + ",";
    json += "\"cursor\":" + std::to_string(cursor) + ",";
    json += "\"playing\":" + std::to_string(playing) + ",";

    json += "\"sequences\":{";

    for (auto it1 = sequence_map.cbegin(); it1 != sequence_map.cend();) {

        Sequence seq = it1->second;

        json += "\"" + seq.address + "\":{";

        json += "\"enabled\":" + std::to_string(seq.enabled) + ",";
        json += "\"note\":" + std::to_string(seq.note) + ",";
        if (seq.note) json += "\"note_on\":" + std::to_string(seq.note_on) + ",";

        json += "\"values\":{" ;

        for (auto it2 = seq.values.cbegin(); it2 != seq.values.cend();) {
            json += "\"" + std::to_string(it2->first) + "\":" + std::to_string(it2->second);
            it2++;
            if (it2 != seq.values.cend()) json += ",";
        }

        json += "}}";

        it1++;
        if (it1 != sequence_map.cend()) json += ",";

    }

    json += "}}";

    fprintf(stderr, "%s", json.c_str());

}
