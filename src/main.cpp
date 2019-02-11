#include <signal.h>
#include <unistd.h>
#include <getopt.h>

#include "jack.hpp"
#include "sequencer.hpp"

#define DEFAULT_OSC_PORT "5244"
#define DEFAULT_TARGET_URL "osc.udp://127.0.0.1:5245"

const char* optstring = "p:t:f:vh";

struct option long_options[] = {
    { "help", 0, 0, 'h' },
    { "osc-port", 1, 0, 'p' },
    { "target", 1, 0, 't' },
    { "feedback", 1, 0, 'f' },
    { "version", 0, 0, 'v' },
    { 0, 0, 0, 0 }
};

static void usage(char *argv0)
{
    fprintf(stderr, "SeqZero2\n\n");

    fprintf(stderr, "Usage: %s [options...]\n", argv0);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -p <str> , --osc-port=<str>      udp inport number or unix socket path for OSC server (default: %s)\n", DEFAULT_OSC_PORT);
    fprintf(stderr, "  -t <str> , --target-url=<str>    osc.udp or osc.unix target url for sequences messages (default: %s)\n", DEFAULT_TARGET_URL);
    fprintf(stderr, "  -f <str> , --feedback-url=<str>  osc.udp or osc.unix target url for sequencer feedback\n");
    fprintf(stderr, "  -h , --help                      this usage output\n");
    fprintf(stderr, "  -v , --version                   show version only\n");
}

struct OptionInfo
{
    OptionInfo() :
        port(DEFAULT_OSC_PORT), target(DEFAULT_TARGET_URL), feedback(),
        show_usage(0), show_version(0) {}

    const char* port;
    const char* target;
    const char* feedback;

    int show_usage;
    int show_version;
};

static void parse_options (int argc, char **argv, OptionInfo & option_info)
{
    int longopt_index = 0;
    int c;

    while ((c = getopt_long (argc, argv, optstring, long_options, &longopt_index)) >= 0) {
        if (c >= 255) {
            break;
        }

        switch (c) {
        case 1:
            /* getopt signals end of '-' options */
            break;
        case 'h':
            option_info.show_usage++;
            break;
        case 'v':
            option_info.show_version++;
            break;
        case 'p':
            option_info.port = optarg;
            break;
        case 't':
            option_info.target = optarg;
            break;
        case 'f':
            option_info.feedback = optarg;
            break;
        default:
            fprintf (stderr, "argument error: %d\n", c);
            option_info.show_usage++;
            break;
        }

        if (option_info.show_usage > 0) {
            break;
        }
    }

    if (option_info.show_usage) {
        usage(argv[0]);
        exit(1);
    }

    if (option_info.show_version) {
        fprintf(stdout, "SeqZero2 version %s\n", "0.0.0");
        exit(0);
    }
}


bool run = true;

void sighandler(int sig)
{
    run = false;
}

int main(int argc, char* argv[])
{

    OptionInfo option_info;

    parse_options (argc, argv, option_info);

    Sequencer * sequencer = new Sequencer(option_info.port, option_info.target, option_info.feedback);

    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGINT, &sighandler);

    while (run) {
        sleep(1);
    }

    delete sequencer;

    return 0;

}
