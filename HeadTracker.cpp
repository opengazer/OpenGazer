#include "HeadTracker.h"
#include "FMatrixAffineCompute.cpp"
#include "utils.h"

// Decide whether two numbers have the same sign
bool issamesign(int a, int b) {
	if ((a > 0 && b < 0) || (a < 0 && b > 0)) {
		return false;
	}

	return true;
}

static double squareNorm(Point point) {
	return Utils::square(point.x) + Utils::square(point.y);
}

static double mean(std::vector<Point> const &vec, double (Point::*prop)) {
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

void HeadTracker::draw(cv::Mat &image) {
	//std::cout << "state: "<< rotX << " " << rotY << " " << atX << " " << atY << std::endl;

	cv::line(image, cv::Point(320, 240), cv::Point(320 + int(atX * 50), 240 + int(atY * 50)), cv::Scalar(255,255,255));

	//for (int i = 0; i < pointTracker.pointCount; i++) {
	// 	cvLine(
	//		image,
	//		cvPoint(pointTracker.currentPoints[i].x, pointTracker.currentPoints[i].y),
	//		cvPoint(pointTracker.origPoints[i].x - xx0 + xx1, pointTracker.origPoints[i].y - yy0 + yy1),
	//		CV_RGB(255,0,0));
	//}

	for (int i = 0; i < pointTracker.pointCount(); i++) {
		cv::line(
			image,
			cv::Point((int)pointTracker.currentPoints[i].x, (int)pointTracker.currentPoints[i].y),
			cv::Point((int)pointTracker.currentPoints[i].x, int(pointTracker.currentPoints[i].y + _depths[i] * 100)),
			cv::Scalar(0,0,255));
	}

	double scale = 10;
	cv::line(image, cv::Point(320, 240), cvPoint(320 + int(rotY * scale), 240 + int(rotX * scale)), cv::Scalar(255,255,255));
}

void HeadTracker::updateTracker() {
	try {
		_depths.resize(pointTracker.pointCount());
		detectInliers(pointTracker.getPoints(&PointTracker::origPoints, true), pointTracker.getPoints(&PointTracker::currentPoints, true));

		std::vector<Point> origPoints = pointTracker.getPoints(&PointTracker::origPoints, false);
		std::vector<Point> currentPoints = pointTracker.getPoints(&PointTracker::currentPoints, false);

		double xx0 = mean(origPoints, &Point::x);
		double yy0 = mean(origPoints, &Point::y);
		double xx1 = mean(currentPoints, &Point::x);
		double yy1 = mean(currentPoints, &Point::y);

		std::vector<double> *fmatrix = computeAffineFMatrix(origPoints, currentPoints);

		if (fmatrix->empty()) {
			//std::cout << "Problem in computeAffineFMatrix" << std::endl;
			return;
		}

		double a = (*fmatrix)[0];
		double b = (*fmatrix)[1];
		double c = (*fmatrix)[2];
		double d = (*fmatrix)[3];
		double e = (*fmatrix)[4];

		// compute the change
		std::vector<double> offsets(pointTracker.pointCount());

		double depthSum = 0.0001;
		double offsetSum = 0.0001;

		for (int i = 0; i < pointTracker.pointCount(); i++) {
			if (pointTracker.status[i]) {
				double xOrig = pointTracker.origPoints[i].x - xx0;
				double yOrig = pointTracker.origPoints[i].y - yy0;
				double xNew = pointTracker.currentPoints[i].x - xx1;
				double yNew = pointTracker.currentPoints[i].y - yy1;
				double x0 = b * xOrig - a * yOrig;
				double x1 = -d * xNew + c * yNew;
				offsets[i] = x0 - x1;
				offsetSum += offsets[i] * offsets[i];
				depthSum += _depths[i] * _depths[i];
			}
		}

		if (pointTracker.areAllPointsActive()) {
			//std::cout << std::endl;
			depthSum = 1.0;
		}

		double depthScale = sqrt(offsetSum / depthSum);

		rotX = c * depthScale / hypot(a, b) / hypot(c, d);
		rotY = d * depthScale / hypot(a, b) / hypot(c, d);

		atX = -(a * c + b * d) / (c * c + d * d); // at = AmpliTwist
		atY = -(a * d - c * b) / (c * c + d * d); // at = AmpliTwist

		// depths
		std::vector<double> newDepths(pointTracker.pointCount());

		for (int i = 0; i < pointTracker.pointCount(); i++) {
			if (pointTracker.status[i]) {
				newDepths[i] = offsets[i] / depthScale;
			}
		}

		if (newDepths[1] > newDepths[2]) {
			rotX = -rotX;
			rotY = -rotY;
			for (int i = 0; i < pointTracker.pointCount(); i++) {
				_depths[i] = -newDepths[i];
			}
		} else {
			for (int i = 0; i < pointTracker.pointCount(); i++) {
				_depths[i] = newDepths[i];
			}
		}


		//	distance
		//double distance1 = 0.0;
		//double distance2 = 0.0;
		//for (int i = 0; i < pointTracker.pointCount; i++) {
		//	if (pointTracker.status[i]) {
		//		distance1 += square(_depths[i] - newDepths[i]);
		//		distance2 += square(_depths[i] + newDepths[i]);
		//	}
		//}

		//for (int i = 0; i < pointTracker.pointCount; i++) {
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
		std::cout << ex.what() << std::endl;
	}

}

std::vector<bool> HeadTracker::detectInliers(std::vector<Point> const &prev, std::vector<Point> const &now, double radius) {
	assert(prev.size() == now.size());

	std::vector<Point> transitions;
	for (int i = 0; i < prev.size(); i++) {
		transitions.push_back(now[i] - prev[i]);
	}

	//std::cout << "xtrans:";
	//for (int i = 0; i < prev.size(); i++) {
	//	std::cout << " " << transitions[i].x;
	//}
	//std::cout << std::endl;

	//std::cout << "ytrans:";
	//for (int i = 0; i < prev.size(); i++) {
	//	std::cout << " " << transitions[i].y;
	//}
	//std::cout << std::endl;

	std::vector<int> closePoints(transitions.size());
	for (int i = 0; i < transitions.size(); i++) {
		closePoints[i] = 0;
		for (int j = 0; j < transitions.size(); j++) {
			if (squareNorm(transitions[i] - transitions[j]) <= Utils::square(radius)) {
				closePoints[i]++;
			}
		}
	}

	int maxIndex = max_element(closePoints.begin(), closePoints.end()) - closePoints.begin();

	std::vector<bool> inliers(transitions.size());
	for (int i = 0; i < transitions.size(); i++) {
		inliers[i] = squareNorm(transitions[i] - transitions[maxIndex]) <= Utils::square(radius);
	}

	for (int i = 0; i < inliers.size(); i++) {
		if (!inliers[i]) {
			pointTracker.status[i] = false;

			pointTracker.currentPoints[i].x = 0.3 * (pointTracker.origPoints[i].x + transitions[maxIndex].x) + 0.7 * pointTracker.currentPoints[i].x;
			//pointTracker.origPoints[i].x + transitions[maxindex].x;

			pointTracker.currentPoints[i].y = 0.3 * (pointTracker.origPoints[i].y + transitions[maxIndex].y)+ 0.7 * pointTracker.currentPoints[i].y;
			//pointTracker.origPoints[i].y + transitions[maxindex].y;
		}
	}

	return inliers;
}

void HeadTracker::predictPoints(double xx0, double yy0, double xx1, double yy1, double rotX, double rotY, double atX, double atY) {
	double maxDiff = 0.0;
	int diffIndex = -1;

	std::vector<Point> points = pointTracker.getPoints(&PointTracker::origPoints, true);

	for (int i = 0; i < points.size(); i++) {
		Point p(points[i].x - xx0, points[i].y - yy0);
		Point p1 = predictPoint(p, _depths[i], xx1, yy1, rotX, rotY, atX, atY);
		Point p2 = predictPoint(p, -_depths[i], xx1, yy1, rotX, rotY, atX, atY);

		double diff1 = fabs(p1.x - pointTracker.currentPoints[i].x) + fabs(p1.y - pointTracker.currentPoints[i].y);
		double diff2 = fabs(p2.x - pointTracker.currentPoints[i].x) + fabs(p2.y - pointTracker.currentPoints[i].y);
		double diff = diff1 > diff2 ? diff2 : diff1;

		// dubious code, I'm not sure about it
		if (!pointTracker.status[i]) {
			//std::cout << "P1 and P2: " << p1.x << ", " << p1.y << " - " << p2.x << ", " << p2.y << std::endl;
			//std::cout << "DEPTH: " << _depths[i] << std::endl;
			//std::cout << "DIFFS: " << diff1 << ", " << diff2 << std::endl;

			if (diff == diff1) {
				pointTracker.currentPoints[i].x = 0 * pointTracker.currentPoints[i].x + 1 * p1.x;
				pointTracker.currentPoints[i].y = 0 * pointTracker.currentPoints[i].y + 1 * p1.y;
				//std::cout << "UPDATED WITH P1 POSITION" << std::endl;
			} else {
				pointTracker.currentPoints[i].x = 0 * pointTracker.currentPoints[i].x + 1 * p2.x;
				pointTracker.currentPoints[i].y = 0 * pointTracker.currentPoints[i].y + 1 * p2.y;
				//std::cout << "UPDATED WITH P2 POSITION" << std::endl;
			}
		}
	}
}

