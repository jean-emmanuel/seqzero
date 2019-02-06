from seqnone import Sequencer

s = Sequencer(
    target='osc.udp://127.0.0.1:5555'
)
s.sequence_add(
    name= 'test',
    length= 192,
    address= '/c3',
    note=True,
    events= {
      0: 1,
      96: 2,
    },
    enabled= True
)

for i in range(1000):
    s.sequence_add(
        name= 'a' + str(i),
        length= 180,
        address= '/c3',
        note=True,
        events= {
          0: 1,
          90: 2,
        },
        enabled=True
    )

s.play()
s.jack_client.gui()
