#include <map>
#include <lo/lo.h>

#include "sequence.hpp"
#include "config.hpp"
#include "jack.hpp"

class Sequencer
{

    public:

        Sequencer(Jack jack, const char* str_url);
        ~Sequencer();

        // Engine

        int sample_rate;
        double period;
        double elapsed_samples;

        // Transport

        float bpm;
        long cursor;
        bool playing;

        void set_period(double period);
        void set_bpm(float bpm);
        void play_current();

        void play();
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

        void send(const char* address, const char* type, double value);

        lo_address url;

};
