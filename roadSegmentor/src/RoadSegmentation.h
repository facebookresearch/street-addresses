/*
 *  Copyright (c) 2017-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 *  This file contains all relevant functions for connecting roads in an image.
 */

#ifndef RoadSegmentation_h
#define RoadSegmentation_h

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iomanip>  //For setprecision
#include <fstream>  //For writing json file
#include "json.hpp" //For creating json
#include <set>      //For set hash used in Thinning methods

using namespace std;
using namespace cv;
using json = nlohmann::json;

// Methods for Thinning
Mat zhangSuenThinning(Mat im);
void thinningZhangSuenIteration(Mat & im, int iter);
Mat guoHallThinning(Mat im);
void thinningGuoHallIteration(Mat & im, int iter);


// Flood Fill algorithms
void roadFloodFill(Mat image, Mat marked, int row, int col, int roadNumber);
void roadFloodFillColor(Mat colorImage, Mat marked, int row, int col, int color[3], int roadNumber);
void findContourPixels(Mat image, Mat marked, int row, int col, vector<Point> & contourPoints);
void floodFillFindConnectedPixels(Mat image, int row, int col, vector<Point> * visited, Mat marked, int maxSize);
void findSpikeLength(Mat image, int row, int col, vector<Point> * linePoints, int spikeLengthThresh);
void connectedPixels(Mat image, int row, int col, vector<Point> * visited, Mat marked);


// Methods for road segmentation
bool isJunction(Mat thinImage, int row, int col);
int returnRoadNumber(Point roadPoint);
void sortIntersections(vector<Point> intersections, Point center, vector<Point> * startPoints, vector<Point> * endPoints, vector<int> * angles);
float pixelDist(Point a, Point b);
int neighbourCount(Mat image, int row, int col);
Mat fillGapsInBinaryImage(Mat bw, int size);
void writeJSON(Mat image, vector<Point> roadLabelsPoints[], string filename);
Mat constructOrderedRoadLabels(Mat labelImage);
Mat binJunctionPixels(Mat thinImage, Mat labelImage);
Mat constructInitialRoadLabels(Mat thinImage);
Mat constructLabelImageFromRoadLabels(Mat thinImage);
vector<Point> findImageCorners(Mat thinImage);
Mat findContinuousRoads(Mat labelImage, vector<Point> cornerVector);
Mat removeSpikes(Mat thinImage, int length);
Mat removeSmallPixels(Mat bw);


// Global variables for counting roads
extern int totalRoadCount;
extern vector<Point> roadLabels[90000];


#endif
