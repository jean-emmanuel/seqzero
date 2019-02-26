#include <cstring>
#include <jack/jack.h>
#include <json.h>

#include "sequencer.hpp"
#include "sequence.hpp"
#include "config.hpp"
#include "jack.hpp"

Sequencer::Sequencer(const char* osc_in_port, const char* osc_target_url, const char* osc_feedback_url, bool _jack_transport)
{

    // Engine
    jack = new Jack(this, "SeqZero", _jack_transport);
    elapsed_time = jack->get_time();

    // Transport

    set_bpm(Config::DEFAULT_BPM);
    jack_transport = _jack_transport;

    // Osc

    osc_port = osc_in_port;
    osc_target = lo_address_new_from_url(osc_target_url);
    osc_feedback_target = lo_address_new_from_url(osc_feedback_url);

    osc_init();

}

Sequencer::~Sequencer()
{

    notes_off();

    for (auto& item: sequence_map) {
        delete item.second;
    }
    sequence_map.clear();

    lo_address_free(osc_target);
    lo_address_free(osc_feedback_target);
    lo_server_thread_free(osc_server);

    delete jack;

}

void Sequencer::set_period(double p)
{

    period = p * 1000000; // microseconds

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
    feed_status();

}

void Sequencer::process()
{

    jack_time_t jack_time = jack->get_time();

    if (!playing) {
        elapsed_time = jack_time;
        return;
    }

    jack_time_t delta = jack_time - elapsed_time;

    int ticks = (int)(delta / period);

    if (ticks > 0) {

        int i;

        for (i = 0; i < ticks; i++) {
            play_current();
        }

        elapsed_time += ticks * period;

    }

}

void Sequencer::play_current()
{

    for (auto& item: sequence_map) {
        item.second->play(cursor);
    }

    feed_status();
    cursor += 1;

}

void Sequencer::notes_off()
{

    for (auto& item: sequence_map) {
        item.second->note_off();
    }

}

void Sequencer::set_cursor(long c, bool from_jack) {

    if (jack_transport && !from_jack) {

        jack->set_cursor(c);

    } else {

        cursor = c;

    }

}

void Sequencer::play(bool from_jack) {

    if (playing) {

        trig(from_jack);

    } else {

        if (jack_transport && !from_jack) {

            jack->play();

        } else {

            playing = true;
            feed_status();

        }


    }

}

void Sequencer::pause(bool from_jack) {

    if (playing) {

        if (jack_transport && !from_jack) {

            jack->pause();

        } else {

            notes_off();
            playing = false;
            feed_status();

        }


    }

}

void Sequencer::stop(bool from_jack) {

    pause(from_jack);
    set_cursor(0, from_jack);

}

void Sequencer::trig(bool from_jack) {

    if (jack_transport && !from_jack) {

        jack->set_cursor(0);
        jack->play();

    } else {

        stop(from_jack);
        play(from_jack);

    }


}

void Sequencer::sequence_add(std::string address, const char* type,
                    std::map<int, double> values, int length, bool enabled, bool is_note)
{

    sequence_map[address] = new Sequence(this, address, type, values, length, enabled, is_note);

}


void Sequencer::sequence_add_json(const char* json_str)
{

    json_object * json = json_tokener_parse(json_str);

    if (!json) return;

    json_object * walker;

    std::string address;
    bool is_note = false;
    bool enabled = false;
    int length = 0;
    const char* osc_type = "f";
    std::map<int, double> values;

    if (json_object_object_get_ex(json, "address", &walker)) {
        address = json_object_get_string(walker);
    }

    if (address.c_str()[0] !=  '/') return;

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


    sequence_add(address, osc_type, values, length, enabled, is_note);

}

void Sequencer::sequence_control(std::string address, int command)
{

    switch(command) {
        case SEQUENCE_ENABLE:
            sequence_map[address]->enable();
            break;
        case SEQUENCE_DISABLE:
            sequence_map[address]->disable();
            break;
        case SEQUENCE_TOGGLE:
            sequence_map[address]->toggle();
            break;
        case SEQUENCE_STATUS:
            sequence_map[address]->feed_status(false);
            break;
        case SEQUENCE_REMOVE:
            std::map<std::string, Sequence *>::iterator it = sequence_map.find(address);
            delete it->second;
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

    osc_server = lo_server_thread_new(osc_port, osc_error);

    if (!osc_server) {
        exit(1);
    }

    lo_server_thread_add_method(osc_server, "/sequencer", NULL, Sequencer::osc_ctrl_handler, this);
    lo_server_thread_add_method(osc_server, "/sequence", "ss", Sequencer::osc_seqctrl_handler, this);

    lo_server_thread_start(osc_server);

    osc_proto = lo_server_get_protocol(lo_server_thread_get_server(osc_server));

}


void Sequencer::osc_send(std::string address, const char* type, double value)
{

    lo_server from = osc_proto == LO_UNIX ? NULL : lo_server_thread_get_server(osc_server);

    if (type[0] == 'i') {
        int ivalue = value;
        lo_send_from(osc_target, from, LO_TT_IMMEDIATE, address.c_str(), type, ivalue);
    } else {
        lo_send_from(osc_target, from, LO_TT_IMMEDIATE, address.c_str(), type, value);
    }

}

void Sequencer::osc_send_feed(std::string address, std::string json)
{

    if (osc_feedback_target) {

        lo_server from = osc_proto == LO_UNIX ? NULL : lo_server_thread_get_server(osc_server);

        lo_send_from(osc_feedback_target, from, LO_TT_IMMEDIATE, address.c_str(), "s", json.c_str());

    }

}

int Sequencer::osc_ctrl_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{

    Sequencer *sequencer = (Sequencer *) user_data;

    std::string command_str = &argv[0]->s;

    int command = sequencer->osc_commands[command_str];

    switch(command) {
        case SEQUENCER_PLAY:
            sequencer->play(false);
            break;
        case SEQUENCER_PAUSE:
            sequencer->pause(false);
            break;
        case SEQUENCER_STOP:
            sequencer->stop(false);
            break;
        case SEQUENCER_TRIG:
            sequencer->trig(false);
            break;
        case SEQUENCER_BPM:
            if (argc > 1) {
                if (types[1] == 'i') sequencer->set_bpm(argv[1]->i);
                if (types[1] == 'f') sequencer->set_bpm(argv[1]->f);
            }
            break;
        case SEQUENCER_CURSOR:
            if (argc > 1 && types[1] == 'i') {
                sequencer->set_cursor(argv[1]->i, false);
            }
            break;
        case SEQUENCER_WRITE:
            if (argc > 1 && types[1] == 's') {
                sequencer->sequence_add_json(&argv[1]->s);
            }
            break;
        case SEQUENCER_STATUS:
            sequencer->feed_status();
            for (auto& item: sequencer->sequence_map) {
                item.second->feed_status(false);
            }
            break;

    }

    return 0;

}

int Sequencer::osc_seqctrl_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{

    Sequencer *sequencer = (Sequencer *) user_data;

    std::string address = &argv[0]->s;
    std::string command_str = &argv[1]->s;

    if (address.c_str()[0] !=  '/') return 0;

    int command = sequencer->osc_seq_commands[command_str];

    if (!command) return 0;

    if (sequencer->sequence_map.find(address) != sequencer->sequence_map.end()) {

        sequencer->sequence_control(address, command);

    } else {

        for (auto item = sequencer->sequence_map.cbegin(); item != sequencer->sequence_map.cend();) {

            std::string item_address = item->second->address;
            item++;

            if (lo_pattern_match(item_address.c_str(), address.c_str())) {

                sequencer->sequence_control(item_address, command);

            }

        }

    }


    return 0;

}

void Sequencer::feed_status() {

    std::string json = "{";

    json += "\"bpm\":" + std::to_string(bpm) + ",";
    json += "\"cursor\":" + std::to_string(cursor) + ",";
    json += "\"playing\":" + std::to_string(playing);

    json += "}";

    osc_send_feed("/status/sequencer", json);

}
