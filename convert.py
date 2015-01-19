#!/usr/bin/python3

import csv
import sqlite3


DATA_VIBRATION = 'DATA.txt'

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

with open(DATA_VIBRATION, 'r') as fh:
    reader = csv.DictReader(fh, fieldnames=FIELDS_VIBRATION, delimiter=',')
    for line in reader:
        # filter criterias
        if float(line['lat']) == 0 or float(line['lon']) == 0:
            # skip lines with no GPS coordinates
            continue

        # convert NMEA 4-digit to 2-digit GPS
        for x in ['lat', 'lon']:
            line[x] = float(line[x][:2]) + float(line[x][2:]) / 60

        # map sensor IDs to an integer
        type_int = { 
                '1.1': 1, '1.2': 2, '1.3': 3, 
                '2.1': 4, '2.2': 5, '2.3': 6, 
                '3': 7
        }
        line['vibration_type'] = type_int[line['vibration_type']]

        # convert line to a tuple and insert the tuple as row into the database
        data = tuple(line[f] for f in FIELDS_VIBRATION)
        c.execute('INSERT INTO vibration VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)', data)

dbconnector.commit()
dbconnector.close()
