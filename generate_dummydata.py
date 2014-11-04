#!/usr/bin/python3

import random

with open('dummydata_vibration.txt', 'w') as fh:
    for i in range(5000):
        data = { "lat": 51 + random.random(),
                 "lon": 13 + random.random(),
                 "height": 40 + random.random(),
                 "timestamp": 20141030143000,
                 "easy": random.choice([0, 1]),
                 "hard": random.choice([0, 1])
        }
        fh.write("%(lat)s,%(lon)s,%(height)s,%(datetime)s,%(easy)s,%(hard)s\n" % data)