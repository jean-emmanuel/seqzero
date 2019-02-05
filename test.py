from seqnone import Sequencer

s = Sequencer()
s.sequence_add(
    name= 'test',
    length= 192,
    events= {
      0: ['/a', 1, 2],
      96: ['/b', 4, 6],
    },
    enabled= True
)

s.play()
s.jack_client.gui()
