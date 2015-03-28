#pragma once

struct Point {
	double x;
	double y;

	Point();
	Point(double x, double y);
	Point(CvPoint2D32f const &point);
	Point(CvPoint const &point);

	double distance(Point other) const;
    double distance2f(cv::Point2f other) const;
	int closestPoint(const std::vector<Point> &points) const;
	void save(CvFileStorage *out, const char *name) const;
	void load(CvFileStorage *in, CvFileNode *node);
	CvPoint cvPoint() const;
	CvPoint2D32f cvPoint32() const;

	void operator=(CvPoint2D32f const &point);
	void operator=(CvPoint const &point);
    void operator=(cv::Point2f const &point);
	Point operator+(const Point &other) const;
	Point operator-(const Point &other) const;
};

std::ostream &operator<< (std::ostream &out, const Point &p);
std::istream &operator>> (std::istream &in, Point &p);
void convert(const Point &point, CvPoint2D32f &p);
void convert(const Point &point, cv::Point2f &p);

