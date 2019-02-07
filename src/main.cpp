#include <unistd.h>

#include "jack.hpp"
#include "sequencer.hpp"
#include "osc.hpp"


int main()
{

    Jack jack = Jack();

    jack.connect();

    Osc osc = Osc("osc.udp://127.0.0.1:5555");

    Sequencer sequencer = Sequencer(jack, &osc);
    sequencer.play();

    while (1) {
        sleep(1);
    }

    // jack.disconnect();

}
