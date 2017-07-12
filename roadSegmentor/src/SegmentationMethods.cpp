/*
 *  Copyright (c) 2017-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 *  This file contains all relevant functions for connecting roads in an image.
 */

#include "RoadSegmentation.h"

bool isJunction(Mat thinImage, int row, int col)
{
    // Method returns a boolean if the thinImage at position (row,col)
    // is a junction or not.

    int count = 0;
    for (int i=row-1; i<=row+1; i++)
    {
        for (int j=col-1; j<=col+1; j++)
        {
            if (i>0 && j>0 && i<=thinImage.rows && j<=thinImage.cols && thinImage.at<uchar>(i,j) == 255)
                count++;
        }
    }
    return count>3;
}

int returnRoadNumber(Point roadPoint)
{
    // retuns the index of the roadLabel vector which
    // contains the roadPoint Point.

    for (int i = 0; i < totalRoadCount; i++)
    {
        if (find(roadLabels[i].begin(), roadLabels[i].end(), roadPoint) != roadLabels[i].end()) // if roadPoint found
        {
            return i; //return road number
        }
    }
    return -1;
}

Mat constructLabelImageFromRoadLabels(Mat thinImage)
{
    // Creates an image with random colors assigned to
    // every road points in the roadLabel vector.

    srand((int)time(NULL));
    Mat labelImage(thinImage.size(), CV_8UC3, Scalar(0,0,0));
    for (int roadNum = 0; roadNum < totalRoadCount; roadNum++)
    {
        int color[3] = {rand() % 200 + 50, rand() % 200 + 50, rand() % 200 + 50};

        if (roadLabels[roadNum].size() > 2)
        {
            for (int pos=0; pos<roadLabels[roadNum].size(); pos++)
            {
                labelImage.at<Vec3b>(roadLabels[roadNum][pos].y, roadLabels[roadNum][pos].x) = Vec3b(color[0], color[1], color[2]);
            }
        }
    }
    return labelImage;
}

Mat constructInitialRoadLabels(Mat thinImage)
{
    // From a binary thinImage, we are constructing the roadLabels vector.
    // Each vector in the roadLabels vector contains a list of all points in the road.

    int roadNumber = 0;
    Mat marked(thinImage.size(), CV_8U, Scalar(0));

    for (int i=0; i<thinImage.rows; i++)
    {
        for (int j=0; j<thinImage.cols; j++)
        {
            if (thinImage.at<uchar>(i,j) == 255 && marked.at<uchar>(i,j) == 0)
            {
                roadFloodFill(thinImage, marked, i, j, roadNumber);
                roadNumber++;
            }
        }
    }
    return constructLabelImageFromRoadLabels(thinImage);
}

Mat binJunctionPixels(Mat thinImage, Mat labelImage)
{
    // Each junction position contains a group of 8 connected junction pixels. This method identifies
    // and assigns the extra junction pixels to different roads.
    // labelImage contains the pixels that have already been binned into specific roads, while thinImage
    // contains all road pixels.

    // Superimpose label image on thin image and identify missing pixels. Pixels with value 255 are missing.
    Mat superimposedThinLabelImage(thinImage.size(), CV_8UC3, Scalar(0,0,0));

    for (int i=0; i<superimposedThinLabelImage.rows; i++)
    {
        for (int j=0; j<superimposedThinLabelImage.cols; j++)
        {
            if (labelImage.at<Vec3b>(i,j) != Vec3b(0,0,0))
            {
                superimposedThinLabelImage.at<Vec3b>(i,j) = labelImage.at<Vec3b>(i,j);
            }
            else if (thinImage.at<uchar>(i,j) == 255)
            {
                int colorSet = 0;

                // Scan through 4 neighbours -> neighbours with dist = 1, and set
                for (int tRow = i-1; tRow <= i+1; tRow++)
                {
                    for (int tCol = j-1; tCol <= j+1; tCol++)
                    {
                        if (labelImage.at<Vec3b>(tRow,tCol) != Vec3b(0,0,0) && pixelDist(Point(tRow,tCol), Point(i,j)) == 1)
                        {
                            superimposedThinLabelImage.at<Vec3b>(i,j) = labelImage.at<Vec3b>(tRow, tCol);
                            colorSet = 1;
                        }
                    }
                }

                // Scan through remaining neighbours with Dist > 1
                for (int tRow = i-1; tRow <= i+1; tRow++)
                {
                    for (int tCol = j-1; tCol <= j+1; tCol++)
                    {
                        if (colorSet == 0 && labelImage.at<Vec3b>(tRow,tCol) != Vec3b(0,0,0) && pixelDist(Point(tRow,tCol), Point(i,j)) > 1)
                        {
                            superimposedThinLabelImage.at<Vec3b>(i,j) = labelImage.at<Vec3b>(tRow, tCol);
                            colorSet = 1;
                        }
                    }
                }
            }
        }
    }
    return superimposedThinLabelImage;
}

Mat constructOrderedRoadLabels(Mat labelImage)
{
    // Using the labelImage RGB image, fill the roadLabel vector with road points in an ordered fashion.
    // This method utilizes the roadFloodFillColor method for filling the roadLabel vector.

    for (int i=0; i<totalRoadCount; i++)
    {
        roadLabels[i].clear();
    }

    Mat marked(labelImage.size(), CV_8U, Scalar(0));
    int roadNumber = 0;
    for (int i=0; i<labelImage.rows; i++)
    {
        for (int j=0; j<labelImage.cols; j++)
        {
            if (labelImage.at<Vec3b>(i,j) != Vec3b(0,0,0) && marked.at<uchar>(i,j) == 0)
            {
                int color[3] = {labelImage.at<Vec3b>(i,j)[0], labelImage.at<Vec3b>(i,j)[1], labelImage.at<Vec3b>(i,j)[2] };
                roadFloodFillColor(labelImage, marked, i, j, color, roadNumber);
                roadNumber++;
            }
        }
    }
    return constructLabelImageFromRoadLabels(labelImage);
}

vector<Point> findImageCorners(Mat thinImage)
{
    // The method finds all the junctions in the image using isJunction() method
    // and then further combine junctions that are very near to each other into a single
    // junction point. A list of all these points is returned.

    //find junctions in the image
    Mat junctionImage(thinImage.size(), CV_8U, Scalar(0));
    for (int i=0; i<thinImage.rows; i++)
    {
        for (int j=0; j<thinImage.cols; j++)
        {
            if (thinImage.at<uchar>(i,j) == 255 && isJunction(thinImage, i, j))
                junctionImage.at<uchar>(i,j) = 255;
        }
    }

    //find best corners from the junctions across which to draw circles
    blur(junctionImage, junctionImage, Size(7,7));
    threshold(junctionImage, junctionImage, 1, 255, CV_THRESH_BINARY);

    struct circleProps
    {
        Point center; int radius;
    } mCircleProps ;

    vector<Point> cornerVector;

    Mat marked(junctionImage.size(), CV_8U, Scalar(0));
    for (int i=0; i<junctionImage.rows; i++)
    {
        for (int j=0; j<junctionImage.cols; j++)
        {
            if (junctionImage.at<uchar>(i,j) == 255 && marked.at<uchar>(i,j) == 0)
            {
                vector<Point> contourPixels;
                findContourPixels(junctionImage, marked, i, j, contourPixels);

                int circle_center_row = 0, circle_center_col = 0;
                for (int pos=0; pos<contourPixels.size(); pos++)
                {
                    circle_center_row += contourPixels[pos].y;
                    circle_center_col += contourPixels[pos].x;
                }
                cornerVector.push_back(Point(circle_center_col/contourPixels.size(), circle_center_row/contourPixels.size()));
            }
        }
    }
    return cornerVector;
}

void sortIntersections(vector<Point> intersections, Point center, vector<Point> * startPoints, vector<Point> * endPoints, vector<int> * angles)
{
    // Given a junction point (Point center) and the list of intersections points (vector<Point> intersections),
    // this code finds the angle subtended by each intersection point with every other intersection point in the list.
    // This angle subtended at the junction point (Point center).
    // The method sorts all the subtended angles in descending order along with the corresponding interstion points.

    vector<int> angle;
    vector<Point> maxAngleWith;

    for (int i = 0; i < intersections.size(); i++)
    {
        int maxAngle = 0; Point maxIntersect;
        //For each Point, find its angle with remaining points
        for (int j = 0; j < intersections.size(); j++)
        {
            Point firstRoadPoint = intersections[i];
            Point secondRoadPoint = intersections[j];
            int firstRoadAngle = atan2((floor)(firstRoadPoint.y - center.y)*(-1), (floor)(firstRoadPoint.x - center.x)) * 180 / CV_PI;
            int secondRoadAngle = atan2((floor)(secondRoadPoint.y - center.y)*(-1), (floor)(secondRoadPoint.x - center.x)) * 180 / CV_PI;

            if (firstRoadAngle < 0)
                firstRoadAngle = firstRoadAngle + 360;
            if (secondRoadAngle < 0)
                secondRoadAngle = secondRoadAngle + 360;

            int diffAngle = secondRoadAngle - firstRoadAngle;
            if (abs(diffAngle) >= 180)
                diffAngle = 360 - abs(diffAngle);
            else if (abs(diffAngle) < 180)
                diffAngle = abs(diffAngle);

            if (diffAngle > maxAngle)
            {
                maxAngle = diffAngle;
                maxIntersect = intersections[j];
            }
        }

        angle.push_back(maxAngle);
        maxAngleWith.push_back(maxIntersect);
    }

    //Sort angle and intersection vectors
    for (int i = 0; i < intersections.size(); i++)
    {
        for (int j = i; j < intersections.size(); j++)
        {
            if (angle[i] < angle[j])
            {
                //Swap inside angle vector
                int tempAngle = angle[i];
                angle[i] = angle[j];
                angle[j] = tempAngle;

                //Swap inside intersection vector
                Point tempPoint = intersections[i];
                intersections[i] = intersections[j];
                intersections[j] = tempPoint;

                //Swap inside maxAngleWith vector
                Point maxAngleWithTemp = maxAngleWith[i];
                maxAngleWith[i] = maxAngleWith[j];
                maxAngleWith[j] = maxAngleWithTemp;
            }
        }
    }

    *startPoints = intersections;
    *endPoints = maxAngleWith;
    *angles = angle;
}

float pixelDist(Point a, Point b)
{
    // Return pixel distance between two Point (s)

    return float(sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)));
}

int neighbourCount(Mat image, int row, int col)
{
    // Returns the number of 8 connected neighbours of a particular pixel
    // at location (row, col) in the image.

    int count = 0;

    if (image.channels() == 1)
    {
        for (int i=row-1; i<=row+1; i++)
        {
            for (int j=col-1; j<=col+1; j++)
            {
                if (i>0 && j>0 && i<=image.rows && j<=image.cols && image.at<uchar>(i,j) != 0)
                    count++;
            }
        }
    }

    else if (image.channels() > 1)
    {
        for (int i=row-1; i<=row+1; i++)
        {
            for (int j=col-1; j<=col+1; j++)
            {
                if (i>0 && j>0 && i<=image.rows && j<=image.cols && image.at<Vec3b>(i,j) != Vec3b(0,0,0))
                    count++;
            }
        }
    }
    return count;
}

Mat fillGapsInBinaryImage(Mat bw, int size)
{
    // Method for filling empty areas in the input road image
    // which have contour area less than input size (Arg-2)

    Mat marked(bw.size(), CV_8U, Scalar(0));
    for (int i=0; i< bw.rows; i++)
    {
        for (int j=0; j < bw.cols; j++)
        {
            if (marked.at<uchar>(i,j) == 0 && bw.at<uchar>(i,j) == 0)
            {
                vector<Point> pixelVector;
                floodFillFindConnectedPixels(bw, i, j, &pixelVector, marked, size);

                if (pixelVector.size() <= size)
                {
                    for (int pos=0; pos<pixelVector.size(); pos++)
                    {
                        bw.at<uchar>(pixelVector[pos].y, pixelVector[pos].x) = 255;
                    }
                }
            }
        }
    }
    return bw;
}

Mat removeSpikes(Mat thinImage, int length)
{
    // This method removes small lines (perpendicular to bigger lines) that
    // arise sometimes due to thinning. Lines with length < Arg2 will be removed.

    cout << "Removing small lines." << endl;
    //Iterate through the image and find line endings. Classified by having just one neighbour.
    if (thinImage.channels() > 1) cvtColor(thinImage, thinImage, CV_BGR2GRAY);

    for (int i = 0; i < thinImage.rows; i++)
    {
        for (int j = 0; j < thinImage.cols; j++)
        {
            if (thinImage.at<uchar>(i, j) == 255 && neighbourCount(thinImage, i, j) == 2) //isLineEnding(thinImage, i, j)
            {
                vector<Point> linePoints;
                findSpikeLength(thinImage, i, j, &linePoints, length);

                if (linePoints.size() <= length)
                {
                    for (int i = 0; i < linePoints.size(); i++)
                    {
                        thinImage.at<uchar>(linePoints[i].y, linePoints[i].x) = 0;
                    }
                }
            }
        }
    }
    return thinImage;
}


Mat removeSmallPixels(Mat bw)
{
    // Removes small blobs from a thresholded image
    Mat marked(bw.size(), CV_8U, Scalar(0));
    for (int i=0; i< bw.rows; i++)
    {
        for (int j=0; j < bw.cols; j++)
        {
            if (marked.at<uchar>(i,j) == 0 && bw.at<uchar>(i,j) == 255)
            {
                vector<Point> pixelVector;
                connectedPixels(bw, i, j, &pixelVector, marked);

                if (pixelVector.size() < 250)
                {
                    for (int pos=0; pos<pixelVector.size(); pos++)
                    {
                        bw.at<uchar>(pixelVector[pos].y, pixelVector[pos].x) = 0;
                    }
                }
            }
        }
    }
    return bw;
}

Mat findContinuousRoads(Mat labelImage, vector<Point> cornerVector)
{
    // Method detects whether 2 roads are continuous using angle segmented by
    // 2 roads at the junction point.
    // Arg 1: Mat labelImage - This is the input RGB road image.
    // Arg 2: vector<Point> cornerVector - A vector of all corner junction pixel locations for the labelImage.

    int lastPercetage = 0;
    for (int pos=0; pos<cornerVector.size(); pos++)
    {
        int radius = 10;

        vector<Point> intersections; //store circle intersections

        //Draw a circle and find intersections.
        for (int angle=0; angle<360; angle++)
        {
            int boundary_row = cornerVector[pos].y + radius*sin(angle * CV_PI/(float)180);
            int boundary_col = cornerVector[pos].x + radius*cos(angle * CV_PI/(float)180);

            for (int iBoundaryRow = boundary_row-1; iBoundaryRow <= boundary_row+1; iBoundaryRow ++)
            {
                for (int iBoundaryCol = boundary_col-1; iBoundaryCol <= boundary_col+1; iBoundaryCol ++)
                {

                    if (iBoundaryCol >= 0 && iBoundaryRow >= 0 && iBoundaryRow < labelImage.rows && iBoundaryCol < labelImage.cols &&
                        labelImage.at<Vec3b>(iBoundaryRow, iBoundaryCol) != Vec3b(0,0,0) &&
                        find(intersections.begin(), intersections.end(), Point(iBoundaryCol, iBoundaryRow)) == intersections.end())
                    {
                        intersections.push_back(Point(iBoundaryCol, iBoundaryRow));
                        angle = angle + 30;
                        goto NEXTANGLE;
                    }
                }
            }
        NEXTANGLE: ;
        }

        //Iterate through the sorted intersections
        vector<Point> startPoints, endPoints; vector<int> angles;
        sortIntersections(intersections, cornerVector[pos], &startPoints, &endPoints, &angles);

        //Color lines that have an angle of 130 +
        vector<Point> covered;
        for (int iter = 0; iter < intersections.size(); iter++)
        {
            int minAngleGap = 130;

            int first_row, first_col, second_row, second_col;
            if (angles[iter] >= minAngleGap &&
                find(covered.begin(), covered.end(), startPoints[iter]) == covered.end() &&
                find(covered.begin(), covered.end(), endPoints[iter]) == covered.end())
            {
                Point firstRoadPoint = startPoints[iter];
                Point secondRoadPoint = endPoints[iter];
                first_row = firstRoadPoint.y; first_col = firstRoadPoint.x;
                second_row = secondRoadPoint.y; second_col = secondRoadPoint.x;

                // Find color at the first point
                int firstColor[3] = { labelImage.at<Vec3b>(first_row, first_col)[0], labelImage.at<Vec3b>(first_row, first_col)[1], labelImage.at<Vec3b>(first_row, first_col)[2] };
                int firstRoadNumber = returnRoadNumber(Point(first_col, first_row));

                // Find color at second point
                int secondColor[3] = { labelImage.at<Vec3b>(second_row, second_col)[0], labelImage.at<Vec3b>(second_row, second_col)[1], labelImage.at<Vec3b>(second_row, second_col)[2] };
                int secondRoadNumber = returnRoadNumber(Point(second_col, second_row));

                //Move all points in secondRoadNumber to points in firstRoadNumber
                if (firstRoadNumber != secondRoadNumber)
                {
                    Point one_front = roadLabels[firstRoadNumber].front();
                    Point one_last = roadLabels[firstRoadNumber].back();
                    Point two_front = roadLabels[secondRoadNumber].front();
                    Point two_last = roadLabels[secondRoadNumber].back();

                    int min = INT_MAX; int pos_one = 0; int pos_two = 0;

                    if (pixelDist(one_front, two_front) < min)
                    {
                        pos_one = 0; pos_two = 0;
                        min = pixelDist(one_front, two_front);
                    }
                    if (pixelDist(one_front, two_last) < min)
                    {
                        pos_one = 0; pos_two = (int)roadLabels[secondRoadNumber].size() - 1;
                        min = pixelDist(one_front, two_last);
                    }
                    if (pixelDist(one_last, two_front) < min)
                    {
                        pos_one = (int)roadLabels[firstRoadNumber].size() - 1; pos_two = 0;
                        min = pixelDist(one_last, two_front);
                    }
                    if (pixelDist(one_last, two_last) < min)
                    {
                        pos_one = (int)roadLabels[firstRoadNumber].size() - 1; pos_two = (int)roadLabels[secondRoadNumber].size() - 1;
                        min = pixelDist(one_last, two_last);
                    }

                    if (min > 20)
                        goto EXIT1;

                    if (pos_one == 0 && pos_two == 0)
                    {
                        reverse(roadLabels[firstRoadNumber].begin(), roadLabels[firstRoadNumber].end());

                        LineIterator lineIt(labelImage, roadLabels[firstRoadNumber].back(), roadLabels[secondRoadNumber].front(), 8);
                        for (int i = 0; i < lineIt.count; i++, ++lineIt)
                            roadLabels[firstRoadNumber].push_back(lineIt.pos());

                        move(roadLabels[secondRoadNumber].begin(), roadLabels[secondRoadNumber].end(), back_inserter(roadLabels[firstRoadNumber]));
                        roadLabels[secondRoadNumber].clear();
                    }
                    else if (pos_one == 0 && pos_two == roadLabels[secondRoadNumber].size() - 1)
                    {
                        reverse(roadLabels[firstRoadNumber].begin(), roadLabels[firstRoadNumber].end());
                        reverse(roadLabels[secondRoadNumber].begin(), roadLabels[secondRoadNumber].end());

                        LineIterator lineIt(labelImage, roadLabels[firstRoadNumber].back(), roadLabels[secondRoadNumber].front(), 8);
                        for (int i = 0; i < lineIt.count; i++, ++lineIt)
                            roadLabels[firstRoadNumber].push_back(lineIt.pos());

                        move(roadLabels[secondRoadNumber].begin(), roadLabels[secondRoadNumber].end(), back_inserter(roadLabels[firstRoadNumber]));
                        roadLabels[secondRoadNumber].clear();
                    }
                    else if (pos_one == roadLabels[firstRoadNumber].size() - 1 && pos_two == 0)
                    {

                        LineIterator lineIt(labelImage, roadLabels[firstRoadNumber].back(), roadLabels[secondRoadNumber].front(), 8);
                        for (int i = 0; i < lineIt.count; i++, ++lineIt)
                            roadLabels[firstRoadNumber].push_back(lineIt.pos());

                        move(roadLabels[secondRoadNumber].begin(), roadLabels[secondRoadNumber].end(), back_inserter(roadLabels[firstRoadNumber]));
                        roadLabels[secondRoadNumber].clear();
                    }
                    else if (pos_one == roadLabels[firstRoadNumber].size() - 1 && pos_two == roadLabels[secondRoadNumber].size() - 1)
                    {
                        reverse(roadLabels[secondRoadNumber].begin(), roadLabels[secondRoadNumber].end());

                        LineIterator lineIt(labelImage, roadLabels[firstRoadNumber].back(), roadLabels[secondRoadNumber].front(), 8);
                        for (int i = 0; i < lineIt.count; i++, ++lineIt)
                            roadLabels[firstRoadNumber].push_back(lineIt.pos());

                        move(roadLabels[secondRoadNumber].begin(), roadLabels[secondRoadNumber].end(), back_inserter(roadLabels[firstRoadNumber]));
                        roadLabels[secondRoadNumber].clear();
                    }
                }

            EXIT1:
                //Color all pixels for the specific roadNumber
                for (int pos_t = 0; pos_t < roadLabels[firstRoadNumber].size(); pos_t++)
                {
                    if (roadLabels[firstRoadNumber][pos_t].y < labelImage.rows && roadLabels[firstRoadNumber][pos_t].x < labelImage.cols)
                    {
                        labelImage.at<Vec3b>(roadLabels[firstRoadNumber][pos_t].y, roadLabels[firstRoadNumber][pos_t].x) = Vec3b(firstColor[0], firstColor[1], firstColor[2]);
                    }
                }
                covered.push_back(startPoints[iter]); covered.push_back(endPoints[iter]);
            }
        }

    }
    return labelImage;
}

void writeJSON(Mat image, vector<Point> roadLabelsPoints[], string filename)
{
    // Method for creating the JSON from the roadLabelsPoints vector arg.
    // The JSON is saved with filename as specified by the third argument.
    // The image resolution is saved in the JSON using the image input.

    vector<vector<int>> roadLabelsNew[90000];

    for (int i = 0; i < totalRoadCount; i++)
    {
        for (int pos = 0; pos < roadLabelsPoints[i].size(); pos++)
        {
            // Some unknown elements are being found in the roadLabels array. Do not add them in the json.
            if (roadLabelsPoints[i][pos].y < image.rows && roadLabelsPoints[i][pos].y < image.cols)
            {
                vector<int> point;
                point.push_back(roadLabelsPoints[i][pos].y);
                point.push_back(roadLabelsPoints[i][pos].x);
                point.push_back(0);

                roadLabelsNew[i].push_back(point);
            }
        }
    }

    json jsonNew;
    //Create id_to_road json first and store in vector<vector<int>>

    json id_to_road;
    for (int i = 0; i < totalRoadCount; i++)
    {
        if (roadLabelsNew[i].size() > 0)
            id_to_road[to_string(i)] = roadLabelsNew[i];
    }

    json pixel_to_id;
    for (int i = 0; i < totalRoadCount; i++)
    {
        for (int pos = 0; pos < roadLabelsNew[i].size(); pos++)
        {
            vector<int> point = roadLabelsNew[i][pos];
            string key = "(" + to_string(point[0]) + ", " + to_string(point[1]) + ")";
            pixel_to_id[key] = i;
        }
    }

    json image_meta;
    image_meta["width"] = image.cols;
    image_meta["height"] = image.rows;

    jsonNew["id_road"] = id_to_road;
    jsonNew["pixel_road"] = pixel_to_id;
    jsonNew["img_meta"] = image_meta;

    string outputJsonFilename = filename;
    ofstream o(outputJsonFilename);
    o << jsonNew << endl;
}
