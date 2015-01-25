# Bike vibrations

We built an Arduino with vibration sensors which can be mounted on a bike. The
collected data then displays on which part of the route the vibration was the
lowest or heaviest, so you can estimate the quality of the street.

## Arduino code
The Arduino code in this repository runs an Arduino Mega with multiple
vibration sensors (3x soft, 3x hard), an IMU and a GPS receiver. 
Every couple of seconds or with any detected vibration all data is collected
and saved into a text file on the Arduino. 

## Python scripts
The Python scripts then convert the raw data from these text files and put the
values in a database, row by row. This gives us a more reliable data source for
other scripts and calculations.

## Interpolation with R
Since the data is only saved every x seconds, the route consists of periodical
measure points. With R we try to interpolate this points to a vector based
route. This data can then be displayed in Openlayers, like some traffic status
maps do.

## Map creation
Extraction to map means getting all measured points from the database and
create a JSON file which then can be imported by Openlayers.

Uses:
* Openlayers 3 http://openlayers.org/
* Red marker icon http://www.clker.com/clipart-red-pin-5.html


Copyright 2015 Georg Bräunig, Dominik Pataky, Felix Wiemann
