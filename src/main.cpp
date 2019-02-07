#include <unistd.h>

#include "jack.hpp"
#include "sequencer.hpp"

int main()
{

    Jack jack = Jack();

    jack.connect();

    Sequencer sequencer = Sequencer(jack, "osc.udp://127.0.0.1:5555");

    const char* address = "/test";
    const char* type = "f";
    std::map<int, double> values;
    bool enabled = true;
    bool is_note = false;
    int length = 192;
    values[0] = 1;
    values[91] = 2;
    sequencer.sequence_add(address, type, values, length, enabled, is_note);

    // sequencer.play();

    while (1) {
        sleep(1);
    }

    // jack.disconnect();

}
