import pyo
import liblo
import json
import re

from .sequence import Sequence
from .config import MIN_BPM, MAX_BPM, DEFAULT_BPM, PPQN

class Sequencer():

    def __init__(self, target, bpm=DEFAULT_BPM):

        self.target = 'osc.udp://127.0.0.1:5555'

        self.jack_client = pyo.Server(audio='jack', nchnls=0, jackname='SeqNone')
        self.jack_client.boot()
        self.jack_client.start()

        self.bpm = bpm
        self.cursor = 0

        self.clock = pyo.Pattern(self.next_tick)

        self.set_bpm(self.bpm)

        self.sequences = {}
        self.active_sequences = []

        self.osc_queue = []

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

        cursor = self.cursor

        for sequence in self.active_sequences:

            sequence.play(cursor)

        if len(self.osc_queue):

            liblo.send(self.target, liblo.Bundle(*self.osc_queue))
            self.osc_queue = []


    def send(self, event):

        self.osc_queue.append(event)


    def sequence_get(self, name):

        sequences = []
        for n in self.sequences:
            if re.match(name, n):
                sequences.push(self.sequences[n])
        return sequences

    def sequence_add(self, **kwargs):

        sequence = Sequence(parent=self, **kwargs)
        self.sequences[sequence.name] = sequence

    def sequence_remove(self, name):

        for s in self.sequence_get(name):
            s.disable()
            del self.sequences[s.name]

    def sequence_enable(self, name):

        for s in self.sequence_get(name):
            s.enable()

    def sequence_disable(self, name):

        for s in self.sequence_get(name):
            s.disable()

    def sequence_toggle(self, name, state):

        for s in self.sequence_get(name):
            s.toggle(state)
