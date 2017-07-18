#!/usr/bin/env python

# Copyright (c) 2017-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
#

import argparse
import json
from xml.dom.minidom import parseString
from rtree import index
from util.utils import haversine, on_segment, get_bounding_box, \
get_closest_point, convert, point_dist_from_start


def get_address_city(path, lat, lon, city):
    """ generates address from lat lon

        Keyword arguments:
        path -- path to data
        lat -- latitude (float)
        lon -- longitude (float)
        city -- name of city
    """
    idx = index.Index(path + '/rtree')

    candidates = list(idx.intersection(
                get_bounding_box(lat, lon), objects=True
            ))

    if not candidates:
        print("No address")
        return (float('inf'), 'No address found :(')

    close_points = [(get_closest_point(can.object[0], can.object[1],
                        (lat, lon)), can.object[0], can.object[1],
                        can.object[2], can.object[3]) for can in candidates]

    ainf = min([(haversine(close_point[0], (lat, lon)),
                        close_point[0], close_point[1],
                        close_point[2], close_point[3], close_point[4])
                        for close_point in close_points], key=lambda x: x[0])

    orth_dist = round(ainf[0]) / 5 + 65
    meter = (haversine(ainf[1], ainf[2]) + ainf[4]) / 5
    name = ainf[5]

    a = ainf[2]
    b = ainf[3]
    sign = ((b[0] - a[0])*(lon - b[1]) - (b[1] - a[1])*(lat - b[0]))

    if sign <= 0:
        meter = int(2 * round(meter/2))

    else:
        meter = int(2 * round(meter/2) + 1)

    print "Adress: " + str(str(meter) + str(chr(int(orth_dist)))
                          + "." + name + "." + city)

    return (orth_dist, str(str(meter) + str(chr(int(orth_dist)))
                          + "." + name + "." + city))


def get_lat_lon(path, meter, block, street):
    """ generates lat lon from address in the form 52b, nc17

        Keyword arguments:
        path -- path to data
        meter -- meter length along road ("52" in above)
        block -- block character away from road ("b" in above)
        street -- street name ("nc17" in above)
    """
    meter = int(meter)
    orth_dist = (ord(block.upper()) - 64.5)*5
    street = street.upper()

    ntr_str = open(path + '/name_to_road.json').read()
    dim_road = json.loads(ntr_str)
    dims = dim_road[0]
    row, col = dims['height'], dims['width']
    name_to_road = dim_road[1]

    with open(path + '/roads.osm', 'r') as f:
        for i in range(3):
            osm = f.readline()
    doc = parseString(osm)
    bounds = doc.getElementsByTagName('bounds')[0]

    convert(bounds.getAttribute('minlat'),
                   bounds.getAttribute('minlon'),
                   bounds.getAttribute('maxlat'),
                   bounds.getAttribute('maxlon'),
                   row, col
                   )
    try:
        road = name_to_road[street]
    except:
        print "Street " + street + " was not found on the current map."
        return None
    prev = None
    curr = (convert.x_to_lat(int(road[0][0])),
            convert.y_to_lon(int(road[0][1])))
    dist = 0
    for i in range(len(road)-1):
        next = (convert.x_to_lat(int(road[i+1][0])),
                 convert.y_to_lon(int(road[i+1][1])))
        edge_dist = haversine(curr, next)/float(5)
        dist += edge_dist
        if dist > meter:
            break
        curr = next

    lat, lon = point_dist_from_start(curr, next, meter - (dist - edge_dist),
                                orth_dist, (meter % 2 != 0))
    print "Lat, Lon: " + str(lat) + ", " + str(lon)
    return lat, lon


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument('-path', '--path', required=True, type=str, help='Path to osm, rtree, and json files.')
    ap.add_argument('-lat', '--lat', type=float, help='Latitude of point')
    ap.add_argument('-lon', '--lon', type=float, help='Longitude of point')
    ap.add_argument('-city', '--city', type=str, help='Name of city')
    ap.add_argument('-meter', '--meter', type=int, help='Meter along road')
    ap.add_argument('-block', '--block', type=str, help='Block from road (a, b, c, etc.)')
    ap.add_argument('-street', '--street', type=str, help='Name of street')
    args = vars(ap.parse_args())
    if args.get('lat') and args.get('lon'):
        get_address_city(args['path'], args['lat'], args['lon'], args['city'])
    elif args.get('meter') and args.get('block') and args.get('street'):
        get_lat_lon(args['path'], args['meter'], args['block'], args['street'])
    else:
        print("Please enter a lat lon, or an address.")
