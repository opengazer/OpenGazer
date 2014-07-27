#include <fstream>

#include "HeadTracker.h"
#include "FMatrixAffineCompute.cpp"

// Decide whether two numbers have the same sign
bool issamesign(int a, int b) {
	if ((a > 0 && b < 0) || (a < 0 && b > 0)) {
		return false;
	}

	return true;
}

static double squareNorm(Point point) {
	return square(point.x) + square(point.y);
}


static double mean(vector<Point> const &vec, double (Point::*prop)) {
	double sum = 0.0;
	for (int i = 0; i < vec.size(); i++) {
		sum += (vec[i].*prop);
	}

	return sum / vec.size();
}


static void predict(double xOrig, double yOrig, double a, double b, double c, double d, double depth, double &xNew, double &yNew) {
	double A = -a * xOrig - b * yOrig;
	double B = depth - b * xOrig + a * yOrig;

	xNew = (c * A + d * B) / (d * d + c * c);
	yNew = (d * A - c * B) / (d * d + c * c);
}

static Point predictPoint(Point p, double depth, double dMeanX, double dMeanY, double rotX, double rotY, double atX, double atY) {
	Point p2 (p.x * atX - p.y * atY, p.x * atY + p.y * atX);

	return Point(p2.x + rotY * depth + dMeanX, p2.y - rotX * depth + dMeanY);
}

HeadTracker::HeadTracker(PointTracker &pointTracker):
	pointTracker(pointTracker)
{
}

void HeadTracker::draw(IplImage *image) {
	//cout << "state: "<< rotX << " " << rotY << " " << atX << " " << atY << endl;

	cvLine(image, cvPoint(320, 240), cvPoint(320 + int(atX * 50), 240 + int(atY * 50)), CV_RGB(255,255,255));

	//for (int i = 0; i < pointTracker.pointcount; i++) {
	// 	cvLine(image, cvPoint(pointTracker.currentpoints[i].x, pointTracker.currentpoints[i].y), cvPoint(pointTracker.origpoints[i].x - xx0 + xx1, pointTracker.origpoints[i].y - yy0 + yy1), CV_RGB(255,0,0));
	//}

	for (int i = 0; i < pointTracker.pointcount(); i++) {
		cvLine(image, cvPoint((int)pointTracker.currentpoints[i].x, (int)pointTracker.currentpoints[i].y), cvPoint((int)pointTracker.currentpoints[i].x, int(pointTracker.currentpoints[i].y + _depths[i] * 100)), CV_RGB(0,0,255));
	}

	double scale = 10;
	cvLine(image, cvPoint(320, 240), cvPoint(320 + int(rotY * scale), 240 + int(rotX * scale)), CV_RGB(255,0,0));
}

void HeadTracker::updateTracker() {
	try {
		_depths.resize(pointTracker.pointcount());
		detectInliers(pointTracker.getpoints(&PointTracker::origpoints, true), pointTracker.getpoints(&PointTracker::currentpoints, true));

		vector<Point> origPoints = pointTracker.getpoints(&PointTracker::origpoints, false);
		vector<Point> currentPoints = pointTracker.getpoints(&PointTracker::currentpoints, false);

		double xx0 = mean(origPoints, &Point::x);
		double yy0 = mean(origPoints, &Point::y);
		double xx1 = mean(currentPoints, &Point::x);
		double yy1 = mean(currentPoints, &Point::y);

		vector<double> *fmatrix = computeAffineFMatrix(origPoints, currentPoints);

		if (fmatrix->empty()) {
			//cout << "Problem in computeAffineFMatrix" << endl;
			return;
		}

		double a = (*fmatrix)[0];
		double b = (*fmatrix)[1];
		double c = (*fmatrix)[2];
		double d = (*fmatrix)[3];
		double e = (*fmatrix)[4];

		// compute the change
		vector<double> offsets(pointTracker.pointcount());

		double depthSum = 0.0001;
		double offsetSum = 0.0001;

		for (int i = 0; i < pointTracker.pointcount(); i++) {
			if (pointTracker.status[i]) {
				double xOrig = pointTracker.origpoints[i].x - xx0;
				double yOrig = pointTracker.origpoints[i].y - yy0;
				double xNew = pointTracker.currentpoints[i].x - xx1;
				double yNew = pointTracker.currentpoints[i].y - yy1;
				double x0 = b * xOrig - a * yOrig;
				double x1 = -d * xNew + c * yNew;
				offsets[i] = x0 - x1;
				offsetSum += offsets[i] * offsets[i];
				depthSum += _depths[i] * _depths[i];
			}
		}

		if (pointTracker.areallpointsactive()) {
			//cout << endl;
			depthSum = 1.0;
		}

		double depthScale = sqrt(offsetSum / depthSum);

		rotX = c * depthScale / hypot(a, b) / hypot(c, d);
		rotY = d * depthScale / hypot(a, b) / hypot(c, d);

		atX = -(a * c + b * d) / (c * c + d * d); // at = AmpliTwist
		atY = -(a * d - c * b) / (c * c + d * d); // at = AmpliTwist

		// depths
		vector<double> newDepths(pointTracker.pointcount());

		for (int i = 0; i < pointTracker.pointcount(); i++) {
			if (pointTracker.status[i]) {
				newDepths[i] = offsets[i] / depthScale;
			}
		}

		if (newDepths[1] > newDepths[2]) {
			rotX = -rotX;
			rotY = -rotY;
			for (int i = 0; i < pointTracker.pointcount(); i++) {
				_depths[i] = -newDepths[i];
			}
		} else {
			for (int i = 0; i < pointTracker.pointcount(); i++) {
				_depths[i] = newDepths[i];
			}
		}


		//	distance
		//double distance1 = 0.0;
		//double distance2 = 0.0;
		//for (int i = 0; i < pointTracker.pointcount; i++) {
		//	if (pointTracker.status[i]) {
		//		distance1 += square(_depths[i] - newDepths[i]);
		//		distance2 += square(_depths[i] + newDepths[i]);
		//	}
		//}

		//for (int i = 0; i < pointTracker.pointcount; i++) {
		//	if (pointTracker.status[i]) {
		//		if (distance1 > distance2) {
		//			rotx = -rotx;
		//			roty = -roty;
		//			_depths[i] = -newDepths[i];
		//		} else {
		//			_depths[i] = newDepths[i];
		//		}
		//	}
		//}

		predictPoints(xx0, yy0, xx1, yy1, rotX, rotY, atX, atY);
	}
	catch (std::exception &ex) {
		cout << ex.what() << endl;
	}

}

vector<bool> HeadTracker::detectInliers(vector<Point> const &prev, vector<Point> const &now, double radius) {
	assert(prev.size() == now.size());

	vector<Point> transitions;
	for (int i = 0; i < prev.size(); i++) {
		transitions.push_back(now[i] - prev[i]);
	}

	//cout << "xtrans:";
	//for (int i = 0; i < prev.size(); i++) {
	//	cout << " " << transitions[i].x;
	//}
	//cout << endl;

	//cout << "ytrans:";
	//for (int i = 0; i < prev.size(); i++) {
	//	cout << " " << transitions[i].y;
	//}
	//cout << endl;

	vector<int> closePoints(transitions.size());
	for (int i = 0; i < transitions.size(); i++) {
		closePoints[i] = 0;
		for (int j = 0; j < transitions.size(); j++) {
			if (squareNorm(transitions[i] - transitions[j]) <= square(radius)) {
				closePoints[i]++;
			}
		}
	}

	int maxIndex = max_element(closePoints.begin(), closePoints.end()) - closePoints.begin();

	vector<bool> inliers(transitions.size());
	for (int i = 0; i < transitions.size(); i++) {
		inliers[i] = squareNorm(transitions[i] - transitions[maxIndex]) <= square(radius);
	}

	for (int i = 0; i < inliers.size(); i++) {
		if (!inliers[i]) {
			pointTracker.status[i] = false;

			pointTracker.currentpoints[i].x = 0.3 * (pointTracker.origpoints[i].x + transitions[maxIndex].x) + 0.7 * pointTracker.currentpoints[i].x;
			//pointTracker.origpoints[i].x + transitions[maxindex].x;

			pointTracker.currentpoints[i].y = 0.3 * (pointTracker.origpoints[i].y + transitions[maxIndex].y)+ 0.7 * pointTracker.currentpoints[i].y;
			//pointTracker.origpoints[i].y + transitions[maxindex].y;
		}
	}

	return inliers;
}


void HeadTracker::predictPoints(double xx0, double yy0, double xx1, double yy1, double rotX, double rotY, double atX, double atY) {
	double maxDiff = 0.0;
	int diffIndex = -1;

	vector<Point> points = pointTracker.getpoints(&PointTracker::origpoints, true);

	for (int i = 0; i < points.size(); i++) {
		Point p(points[i].x - xx0, points[i].y - yy0);
		Point p1 = predictPoint(p, _depths[i], xx1, yy1, rotX, rotY, atX, atY);
		Point p2 = predictPoint(p, -_depths[i], xx1, yy1, rotX, rotY, atX, atY);

		double diff1 = fabs(p1.x - pointTracker.currentpoints[i].x) + fabs(p1.y - pointTracker.currentpoints[i].y);
		double diff2 = fabs(p2.x - pointTracker.currentpoints[i].x) + fabs(p2.y - pointTracker.currentpoints[i].y);
		double diff = diff1 > diff2 ? diff2 : diff1;

		// dubious code, I'm not sure about it
		if (!pointTracker.status[i]) {
			//cout << "P1 and P2: " << p1.x << ", " << p1.y << " - " << p2.x << ", " << p2.y << endl;
			//cout << "DEPTH: " << _depths[i] << endl;
			//cout << "DIFFS: " << diff1 << ", " << diff2 << endl;

			if (diff == diff1) {
				pointTracker.currentpoints[i].x = 0 * pointTracker.currentpoints[i].x + 1 * p1.x;
				pointTracker.currentpoints[i].y = 0 * pointTracker.currentpoints[i].y + 1 * p1.y;
				//cout << "UPDATED WITH P1 POSITION" << endl;
			} else {
				pointTracker.currentpoints[i].x = 0 * pointTracker.currentpoints[i].x + 1 * p2.x;
				pointTracker.currentpoints[i].y = 0 * pointTracker.currentpoints[i].y + 1 * p2.y;
				//cout << "UPDATED WITH P2 POSITION" << endl;
			}
		}
	}
}

