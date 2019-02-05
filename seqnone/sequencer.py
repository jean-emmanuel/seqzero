import pyo
import liblo
import json

from .sequence import Sequence

MIN_BPM = 40
MAX_BPM = 440
PPQN = 192

class Sequencer():

    def __init__(self):

        self.target = 'osc.udp://127.0.0.1:5555'

        self.jack_client = pyo.Server(audio='jack', nchnls=0, jackname='SeqNone')
        self.jack_client.boot()
        self.jack_client.start()

        self.bpm = 440
        self.cursor = 0

        self.clock = pyo.Pattern(self.next_tick)

        self.set_bpm(self.bpm)

        self.sequences = {}

    def next_tick(self):

        self.process()

        self.cursor += 1

    def play(self):

        self.clock.play()

    def pause(self):

        self.clock.stop()

    def trig(self):

        self.cursor = 0

    def set_bpm(self, bpm):

        self.bpm = max(min(float(bpm), MAX_BPM), MIN_BPM)
        self.clock.setTime(60. / self.bpm / PPQN)

    def process(self):

        events = []

        for name in self.sequences:

            sequence = self.sequences[name]

            if sequence.enabled:

                sequence_cursor = self.cursor % sequence.length

                if sequence_cursor in sequence.events:

                    events.append(sequence.events[sequence_cursor])

        if len(events):

            liblo.send(self.target, liblo.Bundle(*events))




    def sequence_add(self, **kwargs):

        sequence = Sequence(**kwargs)
        self.sequences[sequence.name] = sequence
