#include <vnl/algo/vnl_svd.h>

template <class T>

T sum(vector<T> const& vector) {
    T sum = vector[0];

    for(int i=1; i<vector.size(); i++)
	sum += vector[i];

    return sum;
}

Vector computeAffineFMatrix(vector<HomPoint> const& points1,
			    vector<HomPoint> const& points2)
{
    assert(points1.size() == points2.size());

    int n = points1.size();

    Vector centroid(4, 0.0);
    
    for(int i=0; i<n; i++) {
	centroid[0] += points1[i].x();
	centroid[1] += points1[i].y();
	centroid[2] += points2[i].x();
	centroid[3] += points2[i].y();
    }

    centroid /= n;

    Matrix matrix(n, 4);

    for(int i=0; i<n; i++) {
	matrix(i, 0) = points1[i].x() - centroid[0];
	matrix(i, 1) = points1[i].y() - centroid[1];
	matrix(i, 2) = points2[i].x() - centroid[2];
	matrix(i, 3) = points2[i].y() - centroid[3];
    }

//     cout << "fmatrixcompute: " << endl << matrix << endl << endl;

    vnl_svd<double> svd(matrix);

//     cout << "U = " << endl << svd.U() << endl << endl;
//     cout << "W = " << endl << svd.W() << endl << endl;
//     cout << "V = " << endl << svd.V() << endl << endl;

    Vector v = svd.V().get_column(3);

    Vector result(5);

    result.update(v);		// a,b,c,d
    result[4] = inner_product(v, centroid);

    if (result[4] < 0) 
	result = -result;
    


//     cout << "mat: " << v << endl;
//     cout << "test: " << matrix * v << endl;

    return result.normalize();
}
