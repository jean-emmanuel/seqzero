#include <map>
#include <lo/lo.h>

#include "sequence.hpp"
#include "config.hpp"
#include "jack.hpp"

class Sequencer
{

    public:

        Sequencer(Jack jack, const char* osc_in_port, const char* osc_target_url);
        ~Sequencer();

        // Engine

        int sample_rate;
        double period;
        double elapsed_samples;

        static int jack_callback (jack_nframes_t nframes, void *arg);

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

        std::map<std::string, Sequence> sequence_map;

        void sequence_add(std::string address, const char* type, std::map<int, double> values, int length, bool enabled, bool is_note);
        void sequence_control(std::string address, int command);

        // Osc

        lo_address osc_target;
        const char* osc_port;
        lo_server osc_server;

        void osc_init();
        void osc_send(std::string address, const char* type, double value);

        static int osc_play_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
        static int osc_pause_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
        static int osc_stop_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
        static int osc_trig_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
        static int osc_seqctrl_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
        static int osc_seqwrite_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);

        void osc_feed();

        enum {
            SEQ_ENABLE = 1,
            SEQ_DISABLE,
            SEQ_TOGGLE,
            SEQ_REMOVE,
            SEQ_WRITE
        };

        std::map<std::string, int> osc_sequence_commands = {
            {"enable",  SEQ_ENABLE},
            {"disable", SEQ_DISABLE},
            {"toggle",  SEQ_TOGGLE},
            {"remove",  SEQ_REMOVE},
            {"write",   SEQ_WRITE}
        };

};
