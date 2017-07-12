# Copyright (c) 2017-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
#

""" Converts OSM info into road geotiff image """

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals


import cv2
import gdal
import numpy as np
from osgeo import osr
import xml.etree.ElementTree as ET


def main(xml, out_fn):
    """
    :param xml: xml file (.osm extension)
    :param out_fn: absolute path with .tif extn to write
                   geotiff image
    """
    tree = ET.parse(xml)
    root = tree.getroot()

    bounds = root.find('bounds')
    min_lat = float(bounds.get('minlat'))
    min_lon = float(bounds.get('minlon'))
    max_lat = float(bounds.get('maxlat'))
    max_lon = float(bounds.get('maxlon'))

    height = int((max_lat - min_lat) * 19584 / 0.08789)
    width = int((max_lon - min_lon) * 19584 / 0.08789)
    image = np.zeros((height, width), np.uint8)

    nodeInfo = {}

    for node in root.iter('node'):
        nodeInfo[node.get('id')] = [node.get('lat'), node.get('lon')]

    for way in root.iter('way'):
        isHighway = 0
        for tag in way.findall('tag'):
            if tag.get('k') == 'highway':
                isHighway = 1

        if (isHighway == 0):
            continue
        last_pixel_col = None
        last_pixel_row = None

        for nd in way.findall('nd'):
            nodeID = nd.get('ref')
            node = nodeInfo[nodeID]

            pixel_col = int((float(node[1]) - min_lon) / (max_lon - min_lon) * width)
            pixel_row = int((max_lat - float(node[0])) / (max_lat - min_lat) * height)

            if (last_pixel_row is not None and last_pixel_col is not None):
                cv2.line(image, (pixel_col, pixel_row),
                                    (last_pixel_col, last_pixel_row), 255, 5)

            last_pixel_row = pixel_row
            last_pixel_col = pixel_col

    driver_name = 'GTiff'
    driver = gdal.GetDriverByName(str(driver_name))
    outRaster = driver.Create(out_fn, int(width), int(height), 1, gdal.GDT_Byte)
    outRaster.SetGeoTransform((min_lon, (float)(max_lon - min_lon) / width, 0,
                                    max_lat, 0, (float)(min_lat - max_lat) / height))

    outRasterSRS = osr.SpatialReference()
    outRasterSRS.ImportFromEPSG(4326)
    outRaster.SetProjection(outRasterSRS.ExportToWkt())

    outband = outRaster.GetRasterBand(1)
    outband.WriteArray(image)
    outband.FlushCache()
