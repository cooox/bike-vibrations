#!/usr/bin/python3

import json
import os.path
import sqlite3

JSONFILE = 'web/data.js'


dbconnector = sqlite3.connect('data.db')
dbconnector.row_factory = sqlite3.Row
c = dbconnector.cursor()

c.execute('SELECT * FROM vibration')

entries = []
for row in c.fetchall():
    entries.append(dict(row))

with open(JSONFILE, 'w') as fh:
    fh.write('imported_data = ')
    fh.write(json.dumps({'entries': entries}, sort_keys=True, indent=4))