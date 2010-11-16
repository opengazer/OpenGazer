#include "utils.h"

void convert(const Point& point, CvPoint2D32f& p) {
    p.x = point.x;
    p.y = point.y;
}

ostream& operator<< (ostream& out, const Point& p) {
    out << p.x << " " << p.y << endl;
    return out;
}

istream& operator>> (istream& in, Point& p) {
    in >> p.x >> p.y;
    return in;
}

void Point::operator=(CvPoint2D32f const& point) {
    x = point.x; 
    y = point.y;
}

void Point::operator=(CvPoint const& point) {
    x = point.x; 
    y = point.y;
}


double Point::distance(Point other) const {
    return fabs(other.x - x) + fabs(other.y - y);
}

Point Point::operator+(const Point &other) const {
    return Point(x + other.x, y + other.y);
}
    
Point Point::operator-(const Point &other) const {
    return Point(x - other.x, y - other.y);
}
    
void Point::save(CvFileStorage *out, const char* name) const {
    cvStartWriteStruct(out, name, CV_NODE_MAP);
    cvWriteReal(out, "x", x);
    cvWriteReal(out, "y", y);
    cvEndWriteStruct(out);
}

void Point::load(CvFileStorage *in, CvFileNode *node) {
    x = cvReadRealByName(in, node, "x");
    y = cvReadRealByName(in, node, "y");
}

CvPoint Point::cvpoint(void) const {
    return cvPoint(cvRound(x), cvRound(y));
}

CvPoint2D32f Point::cvpoint32(void) const {
    return cvPoint2D32f(x, y);
}

int Point::closestPoint(const vector<Point> &points) const {
    if (points.empty())
	return -1;

    vector<double> distances(points.size());
    transform(points.begin(), points.end(), distances.begin(), 
	      sigc::mem_fun(*this, &Point::distance));
    return min_element(distances.begin(), distances.end()) - distances.begin();
}
