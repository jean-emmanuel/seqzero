# seqzero

Utra-minimalist osc sequencer

*Work in progress...*

Ideas

- ultra-minimalist engine
  - runs with jack only
  - uses jack time and
  - (optional) follow jack transport
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

Run `make` to build

**Usage**

```
Usage: ./seqzero [options...]
Options:
  -p <str> , --osc-port=<str>      udp inport number or unix socket path for OSC server (default: 5244)
  -t <str> , --target-url=<str>    osc.udp or osc.unix target url for sequences messages (default: osc.udp://127.0.0.1:5245)
  -f <str> , --feedback-url=<str>  osc.udp or osc.unix target url for sequencer feedback
  -j <str> , --jack-transport      follow jack transport
  -h , --help                      this usage output
  -v , --version                   show version only
```

**OSC API**

*Sequencer commands:*

- `/sequencer <command>`, where `<command>` can be:
  - `bpm <int|float>`: set bpm (between 40 and 440)
  - `cursor <int>`: set cursor
  - `bypass <int>`: set bypass (doesn't stop playback but prevents messages from being sent)
  - `play`: start playback
  - `pause`: pause playback
  - `stop`: pause and set cursor to 0
  - `trig`: stop and play immediately
  - `status`: send sequencer and sequences' status
  - `write <str>`: write a sequence, `<str>` is a JSON set of properties (see JSON sequence properties)


*Sequences commands:*

- `/sequence <id> <command>`, where `<id>` is a sequence id or glob-pattern and `<command>`:
  - `enable`
  - `disable`
  - `toggle`
  - `remove`

*Feedback:*

- `/status/sequencer <str>`: where `<str>` is a JSON set of properties:
  - `"bpm": <num>`
  - `"cursor": <int>`
  - `"playing": <bool>`
  - `"bypass": <bool>`

- `/status/sequence <str>`: where `<str>` is a JSON set of properties (see JSON sequence properties)

*JSON sequence properties:*

- `"id": "<str>"`: unique, must start with a `/` (required)
- `"address": "<str>"`: must start with a `/` (default: `id`)
- `"enabled": <bool>`: (default: `false`)
- `"type": "<str>"`: osc message value type `i`, `f` or `d` (default: `f`)
- `"note": <bool>`: if `true`, `0` is sent when the sequence is disabled (default: `false`)
- `"length": <int>`: length in ticks (default: `0`)
- `"values": "<str>"`: `{"<int>": <num>, ...}` JSON set (`<int>` = time in ticks, `<num>` = osc message value)
- `"removed": <bool>` (only in feedback messages)
