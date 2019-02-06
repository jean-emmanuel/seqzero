import liblo

class Event(liblo.Message):

    def __init__(self, address, value):

        liblo.Message.__init__(self, address, value)
        self.value = value

class Sequence():

    def __init__(self, parent, name, length, address, events, enabled=False, note=False):

        self.parent = parent
        self.name = name
        self.length = length
        self.address = address

        self.events = {}
        for i in events:
            self.events[i] = Event(self.address, events[i])

        self.note = bool(note)
        self.note_active = False
        if self.note:
            self.note_off = Event(self.address, 0)

        self.enabled = bool(enabled)
        self.toggle(self.enabled)



    def play(self, cursor):
        
        i = cursor % self.length

        if i in self.events:

            self.parent.send(self.events[i])

            if self.note:

                self.note_active = bool(self.events[i].value)

    def enable(self):

        self.toggle(True)

    def disable(self):

        self.toggle(False)

    def toggle(self, state):

        if self.note_active and self.enabled and not state:

            self.parent.send(self.note_off)


        if state and self not in self.parent.active_sequences:
            self.parent.active_sequences.append(self)

        if not state and self in self.parent.active_sequences:
            del self.parent.active_sequences[self.parent.active_sequences.index(self)]

        self.enabled = state
