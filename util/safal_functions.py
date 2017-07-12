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


from osgeo import gdal
from regionCreator import py_pipeline
import sys
import subprocess


class SAFAL(object):
    """
    Provides functionality for processing road geotiff image
    """
    def __init__(self, args, out_fn, out_dir, logger):
        self.args = args
        self.out_fn = out_fn
        self.out_dir = out_dir
        self.logger = logger

    # run road segmentation
    def RoadSegment(self):
        segBin = self.args['roadSeg_bin'] + " "
        if self.args['input_tiff'] is not None:
            pred_img = self.args['input_tiff'] + " "
        else:
            pred_img = self.out_fn + " "
        roadSegCommand = segBin + pred_img + self.out_dir
        output = subprocess.call(['bash','-c', roadSegCommand])
        if output != 0:
            self.logger.error('Road segmentation failed!')
            sys.exit(-1)

    # Run region segmentation
    def RegionProcess(self):
        """
        :return: json that contains name to roads info
        """
        js_fn = self.out_dir + "/roads.json"
        o_dir = self.out_dir
        center_r = self.args['centre_row']
        center_c = self.args['centre_col']
        ntr_json = py_pipeline.main(js_fn, o_dir, self.logger, center_r, center_c)
        return ntr_json

    # Extract bounding box info from input geotiff image
    def cal_gps(self):
        """
        :return: bounding box info
        """
        if self.args['input_tiff'] is not None:
            ds = gdal.Open(self.args['input_tiff'])
            width = ds.RasterXSize
            height = ds.RasterYSize
        else:
            ds = gdal.Open(self.out_fn)
            width = ds.RasterXSize
            height = ds.RasterYSize
        gt = ds.GetGeoTransform()
        minlon = gt[0]
        minlat = gt[3] + width * gt[4] + height * gt[5]
        maxlon = gt[0] + width * gt[1] + height * gt[2]
        maxlat = gt[3]
        return(minlat, maxlat, minlon, maxlon)
