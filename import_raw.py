#!/usr/bin/python3

import csv
import sqlite3
import sys

if len(sys.argv) < 2:
    exit("Filename of datafile is missing!\nUsage: 'python {} <datafile>'".format(sys.argv[0]))

DATA_VIBRATION = sys.argv[1]

# Seq = Sequence number inserted by the Arduino
# vibration_type = 1 (soft vibration), 2 (hard vibration), 3 (timer update)
FIELDS_VIBRATION = ['seq', 'imu_x', 'imu_y', 'imu_z', 
    'lat', 'lon', 'day', 'month', 'year', 'hours', 'minutes', 'seconds', 
    'runtime', 'vibration_type']

dbconnector = sqlite3.connect('data.db')
c = dbconnector.cursor()

c.execute('DROP TABLE IF EXISTS `vibration`')
c.execute('CREATE TABLE `vibration` ('
        'seq INTEGER UNIQUE PRIMARY KEY, '
        'imu_x FLOAT, '
        'imu_y FLOAT, '
        'imu_z FLOAT, '
        'lat FLOAT, '
        'lon FLOAT, '
        'day INTEGER, '
        'month INTEGER, '
        'year INTEGER, '
        'hours INTEGER, '
        'minutes INTEGER, '
        'seconds INTEGER, '
        'runtime INTEGER, '
        'vibration_type INTEGER)')

counter_total = 0
counter_imported = 0

with open(DATA_VIBRATION, 'r') as fh:
    reader = csv.DictReader(fh, fieldnames=FIELDS_VIBRATION, delimiter=',')
    for line in reader:
        counter_total += 1
        # filter criterias
        if float(line['lat']) == 0 or float(line['lon']) == 0:
            # skip lines with no GPS coordinates
            continue

        # convert NMEA 4-digit to 2-digit GPS
        for x in ['lat', 'lon']:
            line[x] = float(line[x][:2]) + float(line[x][2:]) / 60

        # convert line to a tuple and insert the tuple as row into the database
        data = tuple(line[f] for f in FIELDS_VIBRATION)
        c.execute('INSERT INTO vibration VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)', data)
        counter_imported += 1

dbconnector.commit()
dbconnector.close()

print("{} of {} entries imported into database.".format(counter_imported, counter_total))
