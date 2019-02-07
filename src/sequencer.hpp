#include <map>

#include "config.hpp"
#include "jack.hpp"
#include "sequence.hpp"


class Sequencer
{

    public:

        Sequencer(Jack jack);
        ~Sequencer();

        int sample_rate;

        float period;
        float elapsed_samples;

        float bpm;
        int cursor;

        bool playing;

        void set_period(float period);
        void set_bpm(float bpm);
        void play_current();

        void play();
        void stop();
        void trig();


        std::map<std::string, Sequence> sequences;

        void sequence_add(std::string address, std::map<int, float> values);

};
