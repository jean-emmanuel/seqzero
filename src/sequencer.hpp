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

        void play();
        void pause();
        void stop();
        void trig();


        // Sequences

        std::map<const char*, Sequence> sequences;

        void sequence_add(const char* address, const char* type, std::map<int, double> values, int length, bool enabled, bool is_note);
        void sequence_remove(const char* address);
        void sequence_enable(const char* address);
        void sequence_disable(const char* address);
        void sequence_toggle(const char* address);

        // Osc

        lo_address osc_target;
        const char* osc_port;
        lo_server osc_server;

        void osc_init();
        void osc_send(const char* address, const char* type, double value);

        static int osc_play_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
        static int osc_pause_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
        static int osc_stop_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
        static int osc_trig_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);

};
