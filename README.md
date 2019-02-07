# SeqNone

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
