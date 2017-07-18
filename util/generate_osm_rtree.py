# Copyright (c) 2017-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
#

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import argparse
import json
from lxml import etree
import math
from pickle import dump
from rtree import index
import random
import sys
from util.utils import convert, haversine, bbox, parse_roads, rtree_for_way_edges


REGION_LIST = ['residential', 'industrial', 'greenfield', 'farm',
                    'recreation_ground', 'allotments', 'cemetery']


def main(dim_name_road, o_dir, minlat, maxlat, minlon, maxlon, logger):
    """ creates and saves osm file and rtree (.dat, .idx) files from json

        Keyword arguments:
        dim_name_road -- json of dimensions of pixel image and name_to_road json
        o_dir -- output directory for osm and rtree files
        minlat -- minimum latitude
        maxlat -- maximum latitude
        minlon -- minimum longitude
        maxlon -- maximum longitude
        logger -- logger
    """

    row, col = dim_name_road[0]["height"], dim_name_road[0]["width"]
    ntr = dim_name_road[1]

    convert(minlat, minlon, maxlat, maxlon, row, col)
    root = etree.Element("osm", version="0.6", generator="JOSM")
    etree.SubElement(root, "bounds", minlat=str(minlat),
                        minlon=str(minlon),
                        maxlat=str(maxlat),
                        maxlon=str(maxlon))
    i = 1
    id_list = []
    for name, road in ntr.items():
        id_list.append([name])
        length = len(road) - 1
        for index, point in enumerate(road):
            x, y, d = point

            _lat, _lon = convert.x_to_lat(x), convert.y_to_lon(y)
            etree.SubElement(root, "node", id=str(i),
                                lat=str(_lat), lon=str(_lon),
                                version="1")
            id_list[-1].append(str(i))
            i += 1

    for road in id_list:
        way = etree.SubElement(root, "way",
                                id=str(i),
                                version="1")
        for ref in road[1:]:
            etree.SubElement(way, "nd", ref=ref)

        etree.SubElement(way, "tag", k="highway", v="unclassified")
        etree.SubElement(way, "tag", k="name", v=road[0])
        i += 1

    osm_path = o_dir + '/roads.osm'
    with open(osm_path, 'wb') as output_file:
        output_file.write(etree.tostring(root, xml_declaration=True,
                                            encoding='UTF-8', pretty_print=True))
    logger.info('OSM file written successfully at: ' + osm_path)

    nodes, ways = parse_roads(root)
    rtree_for_way_edges(ways, nodes, o_dir)
    logger.info('All processes finished successfully!')
