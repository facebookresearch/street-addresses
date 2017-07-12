# Copyright (c) 2017-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
#

import math
import json
from rtree import index

def haversine((lat1, lon1), (lat2, lon2)):
    """ great circle distance between lat lon points """
    lon1, lat1, lon2, lat2 = map(math.radians, [lon1, lat1, lon2, lat2])

    # haversine formula
    dlon = lon2 - lon1
    dlat = lat2 - lat1
    a = math.sin(dlat / 2)**2 + math.cos(lat1) \
        * math.cos(lat2) * math.sin(dlon / 2)**2
    c = 2 * math.asin(math.sqrt(a))
    r = 6371e3 # Radius of earth in meters
    return c * r


def on_segment((lat1, lon1), (lat2, lon2), (a, b)):
    """ determines if point (a, b) on greatcircle is inside line segement formed
        by lat lons
    """
    if lat1 < lat2:
        return lat1 < a < lat2
    else:
        return lat2 < a < lat1


def get_closest_point((lat1, lon1), (lat2, lon2), (a, b)):
    """ This is an approximation in spherical coordinates which holds at the
        distances we are concerned with. Determines point on line segment formed
        by lat lons closest to point (a, b).
    """
    if lat1==lat2:
        x, y = lat1, b
    elif lon1==lon2:
        x, y = a, lon1
    else:
        m1 = (lon2 - lon1) / (lat2 - lat1)
        m2 = -1 / m1
        x = (m1 * lat1 - m2 * a + b - lon1) / (m1 - m2)
        y = m2 * (x - a) + b
    if on_segment((lat1, lon1), (lat2, lon2), (x, y)):
        return (x, y)
    else:
        dist1 = haversine((lat1, lon1), (a, b))
        dist2 = haversine((lat2, lon2), (a, b))
        if dist1 > dist2:
            return (lat1, lon2)
        else:
            return (lat2, lon2)


def get_bounding_box(lat, lon):
    assert lat >= -90.0 and lat  <= 90.0
    assert lon >= -180.0 and lon <= 180.0

    half_side_in_km = .13
    lat = math.radians(lat)
    lon = math.radians(lon)

    radius  = 6371
    # Radius of the parallel at given latitude
    parallel_radius = radius*math.cos(lat)

    lat_min = lat - half_side_in_km/radius
    lat_max = lat + half_side_in_km/radius
    lon_min = lon - half_side_in_km/parallel_radius
    lon_max = lon + half_side_in_km/parallel_radius
    rad2deg = math.degrees

    return rad2deg(lat_min), rad2deg(lon_min), \
            rad2deg(lat_max), rad2deg(lon_max)


def convert(minlat, minlon, maxlat, maxlon, row, col):
    """ converts x to lat and y to lon according """
    minlat, minlon, maxlat, maxlon, row, col = \
        float(minlat), float(minlon), float(maxlat), \
        float(maxlon), int(row), int(col)
    x_range = maxlat - minlat
    y_range = maxlon - minlon

    def x_to_lat(x):
        return (((-x + row)* x_range) / float(row)) + minlat

    def y_to_lon(y):
        return ((y * y_range) / float(col)) + minlon

    convert.x_to_lat = x_to_lat
    convert.y_to_lon = y_to_lon


def point_dist_from_start((lat1, lon1), (lat2, lon2), dist, orth_dist, odd):
    """ finds the point along road segment which is dist meters from start.
        returns point which is orthogonal to the road segment orth_dist from
        above point
    """
    if lat1 == lat2 and lon1 == lon2:
        return lat1, lon1
    vec1 = [lat2- lat1, lon2 - lon1]
    vec2 = [-vec1[1], vec1[0]]
    norm1 = 5*dist/haversine((lat1, lon1), (lat2, lon2))
    vec1 = list(map(lambda x: norm1 * x, vec1))
    norm2 = orth_dist/(haversine(vec1, vec2))
    vec2 = list(map(lambda x: norm2 * x, vec2))
    if odd:
        return lat1 + vec1[0] + vec2[0], lon1 + vec1[1] + vec2[1]
    return lat1 + vec1[0] - vec2[0], lon1 + vec1[1] - vec2[1]


def bbox(line, radius=0):
    """ bounding box around line of x,y points """
    return min(p[0] for p in line) - radius, \
        min(p[1] for p in line) - radius, \
        max(p[0] for p in line) + radius, \
        max(p[1] for p in line) + radius


def parse_roads(root):
    """ parses nodes and ways of xml root """
    nodes = {}
    ways = []

    for child in root:
        if child.tag == 'node':
            id = int(child.attrib['id'])
            lat = float(child.attrib['lat'])
            lon = float(child.attrib['lon'])
            nodes[id] = (lat, lon)
        elif child.tag == 'way':
            name = child.find("./tag[@k='name']")
            nds = child.findall("./nd")

            if name is not None:
                ways.append((
                    int(child.attrib['id']),
                    [int(nd.attrib['ref']) for nd in nds],
                    name.attrib['v']
                ))

    return nodes, ways


def rtree_for_way_edges(ways, nodes, o_dir):
        """ build an R-tree for all edges of the given ways """
        rtree_idx = index.Rtree(o_dir + '/' + 'rtree')
        iid = 0
        for wi, w in enumerate(ways):
            dist = 0
            nds = w[1]
            name = w[2]
            for i in range(len(nds) - 1):
                n1, n2 = nodes[nds[i]], nodes[nds[i + 1]]

                if n1 != n2:
                    rtree_idx.insert(
                        iid,
                        bbox([n1, n2]),
                        ((n1[0], n1[1]), (n2[0], n2[1]), dist, name)
                    )
                    iid += 1
                    dist += haversine((n1[0], n1[1]), (n2[0], n2[1]))
        return rtree_idx
