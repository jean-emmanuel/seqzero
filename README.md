# seqzero-2

Utra-minimalist osc sequencer

*Work in progress...*

Goals

- learn C++
- replace seq24 in my workflow

Ideas

- ultra-minimalist engine
  - runs with jack only
  - takes audio samples from jack as time base
  - sends osc only, routing or midi conversion is to be handled externally
  - no built-in ui, everything through osc

- sequences are very simple
  - 1 osc address
  - 1 arg type (integer, float or double)
  - 1 value that changes over time (map<tick, value>)
  - enabled state bool
  - an optional "note" mode (ensures a note off is sent when the sequence is stopped)

- gui : ideally, something like seq24's
  - communicates with the engine using osc
  - handles all the sequence grouping logic
  - 1 sequence group = pianoroll (1 sequence for each non-empty note) + controls (1 sequence per line); grouped sequences would share the same address prefix


**Build**

Requires: `liblo`, `libjson-c`, `libstdc++`, `libjack`

`apt install libstdc++-8-dev liblo-dev libjson-c-dev libjack-jackd2-dev`

Run `./make` to build

**Usage**

```
Usage: ./seqzero2 [options...]
Options:
  -p <str> , --osc-port=<str>      udp inport number or unix socket path for OSC server (default: 5244)
  -t <str> , --target-url=<str>    osc.udp or osc.unix target url for sequences messages (default: osc.udp://127.0.0.1:5245)
  -f <str> , --feedback-url=<str>  osc.udp or osc.unix target url for sequencer feedback
  -h , --help                      this usage output
  -v , --version                   show version only
```

**OSC API**

Transport commands:

- `/sequencer <command>`, where `<command>` can be:
  - `bpm <num>`
  - `cursor <int>`
  - `play`
  - `pause`
  - `stop`
  - `trig`


Sequences commands:

- `/sequence <address> <command>`, where `<address>` is a sequence's address or glob-pattern and `<command>`:
  - `enable`
  - `disable`
  - `toggle`
  - `remove`

Sequence writing:

- `/sequence <address> write <str>`, where `<str` is a json set of properties
  - `"enabled": <bool>`
  - `"type": "<str>"`: osc message value type (`i`, `f` or `d`)
  - `"note": <bool>`: if `true`, `0` is sent when the sequence is disabled
  - `"length": <int>`: length in ticks
  - `"values": "<str>"`: `{"<int>": <num>, ...}` json set (`<int>` = time in ticks, `<num>` = osc message value)

Note: glob-patter is not supported here

Feedback:

- `/status/sequencer <str>`: where `<str>` is a json set of properties:
  - `"bpm": <num>`
  - `"cursor": <int>`
  - `"playing": <bool>`

- `/status/sequence <str>`: where `<str>` is a json set of properties:
  - `"address": "<str>"`
  - `"enabled": <bool>`
  - `"note": <bool>`
  - `"values": {"<int>": <num>, ...}`
  - `"length": <int>`
  - `"removed": <bool>`
