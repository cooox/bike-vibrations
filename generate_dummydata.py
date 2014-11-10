#!/usr/bin/python3

import random


def r(basecoord):
    # return random coordinates around a base coordinate
    return basecoord + random.choice([random.random(), -random.random()]) * 0.001

with open('dummydata_vibration.txt', 'w') as fh:
    for i in range(30):
        data = { "lat": r(51.0294287),
                 "lon": r(13.7235668),
                 "height": 40 + random.random(),
                 "timestamp": 20141030143000,
                 "easy": random.choice([0, 1]),
                 "hard": random.choice([0, 1])
        }
        fh.write("%(lat)s,%(lon)s,%(height)s,%(timestamp)s,%(easy)s,%(hard)s\n" % data)