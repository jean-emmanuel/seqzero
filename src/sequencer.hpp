#include <map>
#include <lo/lo.h>

#include "sequence.hpp"
#include "config.hpp"
#include "jack.hpp"

class Sequencer
{

    public:

        Sequencer(const char* osc_in_port, const char* osc_target_url, const char* osc_feedback_url);
        ~Sequencer();

        // Engine

        double period;
        jack_time_t elapsed_time;

        void process();

        Jack * jack;

        // Transport

        float bpm;
        long cursor;
        bool playing;

        void set_period(double period);
        void set_bpm(float bpm);
        void play_current();
        void notes_off();

        void play();
        void pause();
        void stop();
        void trig();


        // Sequences

        std::map<std::string, Sequence *> sequence_map;

        void sequence_add(std::string address, const char* type, std::map<int, double> values, int length, bool enabled, bool is_note);
        void sequence_add_json(const char* json);
        void sequence_control(std::string address, int command);

        // Osc

        lo_address osc_target;
        lo_address osc_feedback_target;
        const char* osc_port;
        lo_server osc_server;

        void osc_init();
        void osc_send(std::string address, const char* type, double value);
        void osc_send_feed(std::string address, std::string json);

        static int osc_ctrl_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
        static int osc_seqctrl_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);

        void feed_status();

        enum {
            SEQUENCER_ZERO = 0,
            SEQUENCER_CURSOR,
            SEQUENCER_BPM,
            SEQUENCER_PLAY,
            SEQUENCER_PAUSE,
            SEQUENCER_STOP,
            SEQUENCER_TRIG,
            SEQUENCER_STATUS,
            SEQUENCER_WRITE,

            SEQUENCE_ENABLE,
            SEQUENCE_DISABLE,
            SEQUENCE_TOGGLE,
            SEQUENCE_REMOVE,
            SEQUENCE_STATUS
        };

        std::map<std::string, int> osc_commands = {
            {"bpm",     SEQUENCER_BPM},
            {"cursor",  SEQUENCER_CURSOR},
            {"play",    SEQUENCER_PLAY},
            {"pause",   SEQUENCER_PAUSE},
            {"stop",    SEQUENCER_STOP},
            {"trig",    SEQUENCER_TRIG},
            {"status",  SEQUENCER_STATUS},
            {"write",   SEQUENCER_WRITE},
        };

        std::map<std::string, int> osc_seq_commands = {
            {"enable",  SEQUENCE_ENABLE},
            {"disable", SEQUENCE_DISABLE},
            {"toggle",  SEQUENCE_TOGGLE},
            {"remove",  SEQUENCE_REMOVE},
            {"status",  SEQUENCE_STATUS}
        };

};
