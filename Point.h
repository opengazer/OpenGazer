struct Point {
    double x, y;

    Point(): x(0.0), y(0.0) {}
    Point(double x, double y): x(x), y(y) {}
    Point(CvPoint2D32f const& point): x(point.x), y(point.y) {}
    Point(CvPoint const& point): x(point.x), y(point.y) {}

    void operator=(CvPoint2D32f const& point);
    void operator=(CvPoint const& point);

    double distance(Point other) const;
    Point operator+(const Point &other) const;
    Point operator-(const Point &other) const;

    void save(CvFileStorage *out, const char* name) const;
    void load(CvFileStorage *in, CvFileNode *node);

    CvPoint cvpoint(void) const;
    CvPoint2D32f cvpoint32(void) const;

    int closestPoint(const vector<Point> &points) const;
};

ostream& operator<< (ostream& out, const Point& p);
istream& operator>> (istream& in, Point& p);
void convert(const Point& point, CvPoint2D32f& p);

