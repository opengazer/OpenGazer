#pragma once

class LeastSquares {
public:
	const int n;

	LeastSquares(int n);
	void addSample(double xs[], double y);
	void addSample(double x1, double x2, double y);
	void addSample(double x1, double x2, double x3, double y);
	CvMat* solve(void);
	void solve(double &a0, double &a1, double &a2);

private:
	CvMat *X;
	CvMat *Y;
};

