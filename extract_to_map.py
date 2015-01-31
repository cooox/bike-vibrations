#!/usr/bin/python3

import json
from operator import itemgetter
import os.path
import sqlite3

# The target javascript file
JSONFILE = 'web/data.js'

# Amount of digits to use for rounding the GPS coordinates (see filters below)
ROUNDDIGITS = 3


dbconnector = sqlite3.connect('data.db')
dbconnector.row_factory = sqlite3.Row
c = dbconnector.cursor()
c.execute('SELECT * FROM vibration')

# Get all entries from the database
entries = []
for row in c.fetchall():
    entries.append(dict(row))

# Sort list by vibration_type
entries_sorted = sorted(entries, key=itemgetter('vibration_type'))

# List of lists with filtered and cleaned up entries
entries_cleaned = []

print("Coordinates are going to be rounded by {} digits".format(ROUNDDIGITS))

# The following groups result from 3 parts of the same vibration sensor types.
# E.g. 'vibration_type=1' is the soft sensor, as is 'vibration_type=2' etc.
for rge in [[1, 2, 3], [4, 5, 6], [7]]:
    print("Going to clean up all entries for group {}".format(rge))

    # List of all entries of this group
    tmp_entries_per_type = []

    # List of all entries of this group, but filtered (see below)
    tmp_entries_per_type_cleaned = []

    # Filter to find all group entries
    for e in entries_sorted:
        if e['vibration_type'] in rge:
            tmp_entries_per_type.append(e)

    old_count = len(tmp_entries_per_type)
    print("Group {} has {} entries".format(rge, old_count))
    print("Looking for very similar coordinates in this group..")

    # Compares all coordinates in this group entry by entry and filters them.
    # First rounds take long, but the list gets very small at the end.
    for e in tmp_entries_per_type:
        e_lon = e['lon']
        e_lat = e['lat']

        # Save the current entry, as it will be deleted in the next step
        tmp_e = e

        for x in tmp_entries_per_type:
            if round(x['lon'], ROUNDDIGITS) == round(e_lon, ROUNDDIGITS) and round(x['lat'], ROUNDDIGITS) == round(e_lat, ROUNDDIGITS):
                # Here all entries with very similar coordinates like the one
                # we saved will be deleted, including 'e'
                tmp_entries_per_type.remove(x)

        # Append the saved entry to a new list. This means, only one entry from
        # a group of entries with the same coordinates will be added into the
        # javascript data source.
        tmp_entries_per_type_cleaned.append(tmp_e)

    new_count = len(tmp_entries_per_type_cleaned)
    print("Cleaned up group {}, has now {} entries, {} less than before\n".format(rge, new_count, old_count-new_count))
    entries_cleaned += tmp_entries_per_type_cleaned

# Open the JSON file and insert preamble and lists. The file will be included
# as a Javascript source later. See web/index.html
with open(JSONFILE, 'w') as fh:
    fh.write('imported_data = ')
    fh.write(json.dumps({'entries': entries_cleaned}, sort_keys=True, indent=4))
