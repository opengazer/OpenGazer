#include"PointTracker.h"


vector<double>* computeAffineFMatrix(vector<Point> const& points1,
			    vector<Point> const& points2)
{
    assert(points1.size() == points2.size());

    int n = points1.size();

    CvMat* centroid = cvCreateMat(4,1,CV_32FC1);
    cvZero(centroid);
    
    for(int i=0; i<n; i++) {
        CV_MAT_ELEM( *centroid, float, 0, 0 ) += points1[i].x;
        CV_MAT_ELEM( *centroid, float, 1, 0 ) += points1[i].y;
        CV_MAT_ELEM( *centroid, float, 2, 0 ) += points2[i].x;
        CV_MAT_ELEM( *centroid, float, 3, 0 ) += points2[i].y;
    }

    for(int i=0; i<4; i++) {
        CV_MAT_ELEM( *centroid, float, i, 0 ) = CV_MAT_ELEM( *centroid, float, i, 0 ) / n;
    }

    CvMat* matrix = cvCreateMat(n,4,CV_32FC1);
    cvZero(matrix);

    for(int i=0; i<n; i++) {
	CV_MAT_ELEM( *matrix, float, i, 0 ) = points1[i].x - CV_MAT_ELEM( *centroid, float, 0, 0 );
	CV_MAT_ELEM( *matrix, float, i, 1 ) = points1[i].y - CV_MAT_ELEM( *centroid, float, 1, 0 );
	CV_MAT_ELEM( *matrix, float, i, 2 ) = points2[i].x - CV_MAT_ELEM( *centroid, float, 2, 0 );
	CV_MAT_ELEM( *matrix, float, i, 3 ) = points2[i].y - CV_MAT_ELEM( *centroid, float, 3, 0 );
    }

	if(n == 0) {
		return new vector<double>();
	}

    CvMat* Ut  = cvCreateMat(n,n,CV_32FC1);
    CvMat* Dt  = cvCreateMat(n,4,CV_32FC1);
    CvMat* Vt  = cvCreateMat(4,4,CV_32FC1);
    
    cvSVD(matrix, Dt, Ut, Vt, CV_SVD_U_T);

//     cout << "U = " << endl << svd.U() << endl << endl;
//     cout << "W = " << endl << svd.W() << endl << endl;
//     cout << "V = " << endl << svd.V() << endl << endl;

    //Vector v = svd.V().get_column(3);

    vector<double>* result = new vector<double>();
    
    for(int i=0; i<4; i++) {
        result->push_back(CV_MAT_ELEM( *Vt, float, i, 3));
    }
    
    float dot_product = 0;
    for(int i=0; i<4; i++) {
        dot_product += CV_MAT_ELEM( *Vt, float, i, 3) * CV_MAT_ELEM( *centroid, float, i, 0);
    }
    result->push_back(dot_product);
    
    if (dot_product < 0) {
        for(int i=0; i<5; i++) {
            (*result)[i] *= -1;
        }
    }
    
    float length = 0;
    
    for(int i=0; i<5; i++) {
        length += pow((*result)[i], 2);
    }
    
    length = sqrt(length);
    
    for(int i=0; i<5; i++) {
        (*result)[i] = (*result)[i] / length;
    }
   
//     cout << "mat: " << v << endl;
//     cout << "test: " << matrix * v << endl;

    return result;
}


