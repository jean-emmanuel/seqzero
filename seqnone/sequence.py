import liblo

class Sequence():

    def __init__(self, name, length, events, enabled=False):

        self.name = name
        
        self.enabled = bool(enabled)

        self.length = length

        self.events = {}

        for i in events:

            self.events[i] = liblo.Message(*events[i])
