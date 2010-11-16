#pragma once

#include <opencv/cv.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <vnl/algo/vnl_svd.h>
#include <gdkmm.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

//#define DEBUG

using namespace std;
using namespace boost;

#define xforeach(iter,container) \
    for(typeof(container.begin()) iter = container.begin();	\
	iter != container.end(); iter++)

#define xforeachback(iter,container) \
    for(typeof(container.rbegin()) iter = container.rbegin();	\
	iter != container.rend(); iter++)

typedef vnl_vector<double> Vector;
typedef vnl_matrix<double> Matrix;



template <class T>
inline T square(T a) {
    return a * a;
}

template <class T> 
ostream& operator<< (ostream& out, vector<T> const& vec) {
    out << vec.size() << endl;
    xforeach(iter, vec)
	out << *iter << endl;
    return out;
}

template <class T>
istream& operator>> (istream& in, vector<T> &vec) {
    int size;
    T element;

    vec.clear();
    in >> size;
    for(int i=0; i<size; i++) {
	in >> element;
	vec.push_back(element);
    }
    return in;
}


template <class T>
T teefunction(T source, char* prefix, char *postfix="\n") {
    cout << prefix << source << postfix;
    return source;
}

#define debugtee(x) teefunction(x, #x ": ")

template <class T>
void savevector(CvFileStorage *out, const char* name, vector<T>& vec) {
    cvStartWriteStruct(out, name, CV_NODE_SEQ);
    for(int i=0; i<vec.size(); i++) 
	vec[i].save(out);
    cvEndWriteStruct(out);
}

template <class T>
vector<T> loadvector(CvFileStorage *in, CvFileNode *node) {
    CvSeq *seq = node->data.seq;
    CvSeqReader reader;
    cvStartReadSeq(seq, &reader, 0);
    vector<T> result(seq->total);
    for(int i=0; i<seq->total; i++) {
	CvFileNode *item = (CvFileNode*) reader.ptr;
	result[i].load(in, item);
	CV_NEXT_SEQ_ELEM(seq->elem_size, reader);       
    }
    return result;
}

#include <gtkmm.h>

#include "Point.h"

template <class From, class To>
void convert(const From& from, To& to) {
    to = from;
}

template <class From, class To>
void convert(const vector<From> &from, vector<To> &to) {
    to.resize(from.size());
    for(int i=0; i< (int) from.size(); i++)
	convert(from[i], to[i]);
}

class ConstancyDetector {
    int value;
    int counter; 
    int maxcounter;
public:
    ConstancyDetector(int maxcounter) : 
	value(-1), counter(0), maxcounter(maxcounter) {}

    bool isStable(void) {
	return counter >= maxcounter;
    }

    bool isStableExactly(void) {
	return counter == maxcounter;
    }

    bool observe(int newvalue) {
	if (newvalue != value || newvalue < 0) 
	    counter = 0;
	else
	    counter++;
	value = newvalue;
	return isStable();
    }

    void reset(void) {
	counter = 0;
	value = -1;
    }
};

// #define output(X) { cout << #X " = " << X << endl; }


template <class T> 
int maxabsindex(T const& vec, int size) {
    int maxindex = 0;
    for(int i=0; i<size; i++)
	if (fabs(vec[i]) > fabs(vec[maxindex]))
	    maxindex = i;
    return maxindex;
}

namespace boost {
    template <>
	void checked_delete(IplImage *image);
}

void releaseImage(IplImage *image);
shared_ptr<IplImage> createImage(const CvSize &size, int depth, int channels);

typedef shared_ptr<const IplImage> SharedImage;
