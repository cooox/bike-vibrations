#!/usr/bin/python3

import csv
import sqlite3


DATA_VIBRATION = 'dummydata_vibration.txt'
FIELDS_VIBRATION = ['lat', 'lon', 'height', 'datetime', 'easy', 'hard']

dbconnector = sqlite3.connect('data.db')
c = dbconnector.cursor()

c.execute('DROP TABLE IF EXISTS `vibration`')
c.execute('CREATE TABLE `vibration` ('
          'lat FLOAT, '
          'lon FLOAT, '
          'height FLOAT, '
          'DATETIME INTEGER, '
          'easy INTEGER, '
          'hard INTEGER)')

with open(DATA_VIBRATION, 'r') as fh:
    reader = csv.DictReader(fh, fieldnames=FIELDS_VIBRATION, delimiter=',')
    for line in reader:
        c.execute('INSERT INTO vibration VALUES (?,?,?,?,?,?)', (
            line['lat'], line['lon'], line['height'], line['datetime'], line['easy'], line['hard']
        ))

dbconnector.commit()
dbconnector.close()