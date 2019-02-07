#include <unistd.h>

#include "jack.hpp"
#include "sequencer.hpp"


int main()
{

    Jack jack = Jack();

    jack.connect();

    Sequencer sequencer = Sequencer(jack);


    while (1) {
        sleep(1);
    }

    // jack.disconnect();

}
