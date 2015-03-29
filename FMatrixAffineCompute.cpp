#include "Point.h"

std::vector<double> *computeAffineFMatrix(std::vector<Point> const &points1, std::vector<Point> const &points2) {
	assert(points1.size() == points2.size());

	int n = points1.size();

	CvMat *centroid = cvCreateMat(4, 1, CV_32FC1);
	cvZero(centroid);

	for (int i = 0; i < n; i++) {
		CV_MAT_ELEM(*centroid, float, 0, 0) += points1[i].x;
		CV_MAT_ELEM(*centroid, float, 1, 0) += points1[i].y;
		CV_MAT_ELEM(*centroid, float, 2, 0) += points2[i].x;
		CV_MAT_ELEM(*centroid, float, 3, 0) += points2[i].y;
	}

	for (int i = 0; i < 4; i++) {
		CV_MAT_ELEM(*centroid, float, i, 0)  = CV_MAT_ELEM(*centroid, float, i, 0) / n;
	}

	CvMat *matrix = cvCreateMat(n, 4, CV_32FC1);
	cvZero(matrix);

	for (int i = 0; i < n; i++) {
		CV_MAT_ELEM(*matrix, float, i, 0) = points1[i].x - CV_MAT_ELEM(*centroid, float, 0, 0);
		CV_MAT_ELEM(*matrix, float, i, 1) = points1[i].y - CV_MAT_ELEM(*centroid, float, 1, 0);
		CV_MAT_ELEM(*matrix, float, i, 2) = points2[i].x - CV_MAT_ELEM(*centroid, float, 2, 0);
		CV_MAT_ELEM(*matrix, float, i, 3) = points2[i].y - CV_MAT_ELEM(*centroid, float, 3, 0);
	}

	if (n == 0) {
		return new std::vector<double>();
	}

	CvMat *ut  = cvCreateMat(n, n, CV_32FC1);
	CvMat *dt  = cvCreateMat(n, 4, CV_32FC1);
	CvMat *vt  = cvCreateMat(4, 4, CV_32FC1);

	cvSVD(matrix, dt, ut, vt, CV_SVD_U_T);

	//cout << "U = " << endl << svd.U() << endl << endl;
	//cout << "W = " << endl << svd.W() << endl << endl;
	//cout << "V = " << endl << svd.V() << endl << endl;

	//std::vector v = svd.V().get_column(3);

	std::vector<double> *result = new std::vector<double>();

	for (int i = 0; i < 4; i++) {
		result->push_back(CV_MAT_ELEM(*vt, float, i, 3));
	}

	float dotProduct = 0;
	for (int i = 0; i < 4; i++) {
		dotProduct += CV_MAT_ELEM(*vt, float, i, 3) * CV_MAT_ELEM(*centroid, float, i, 0);
	}
	result->push_back(dotProduct);

	if (dotProduct < 0) {
		for (int i = 0; i < 5; i++) {
			(*result)[i] *= -1;
		}
	}

	float length = 0;
	for (int i = 0; i < 5; i++) {
		length += pow((*result)[i], 2);
	}

	length = sqrt(length);

	for (int i = 0; i < 5; i++) {
		(*result)[i] = (*result)[i] / length;
	}

	//cout << "mat: " << v << endl;
	//cout << "test: " << matrix * v << endl;

	return result;
}


