#pragma once
#include "utils.h"

using namespace std;
using namespace boost;

struct StateNode {
    int minduration, maxduration;
    double threshold;
    StateNode(int minduration, int maxduration, double threshold);
    bool isSatisfied(double value) const;
};

class LinearStateSystem {
    const vector<StateNode> states;
    int currentState, duration;
public:
    LinearStateSystem(const vector<StateNode> &states);
    void updateState(double value);
    void setState(int state);
    int getState();
    bool isFinalState();
};

class LambdaAccumulator {
    const double lambda;
    double current;
 public:
    LambdaAccumulator(double lambda, double initial);
    void update(double value);
    double getValue();
};

class BlinkDetector {
    scoped_ptr<IplImage> averageEye;
    LambdaAccumulator acc;
    LinearStateSystem states;
    static vector<StateNode> constructStates();
 public:
    BlinkDetector();
    void update(const scoped_ptr<IplImage> &image);
    int getState();
};



/* template <class T> */
/* class TwoClassDetector { */
/*     const T class0, class1; */
/*  public: */
/*     TwoClassDetector(const T& class0, const T& class1):  */
/*     class0(class0), class1(class1) {} */
	
/*     double classify(const T& object) { */
/* 	double dist0 = distance(class0, object); */
/* 	double dist1 = distance(class0, object); */
/* 	return dist0 / (dist0 + dist1); */
/*     }; */
/* }; */

/* template <class T, class Func> */
/* class Accumulator { */
/*     T accumulator; */
/*     Func func; */
/*  public: */
/*     Accumulator(const T& initial, const Func& func):  */
/*     accumulator(initial), func(func) */
/*     {}; */
/*     void update(const T& object) { */
/* 	accumulator = func(accumulator, object); */
/*     } */
/*     T get() { */
/* 	return accumulator; */
/*     } */
/* }; */

/* template <T> */
/* class GetFurtherObject { */
/*     T referenceobject; */
/*  public: */
/*     T operator()(const T& one, const T& two) { */
/* 	if (distance(referenceobject, one) > distance(referenceobject, two)) */
/* 	    return one; */
/* 	else */
/* 	    return two; */
/*     } */
/* }; */

/* Accumulator<shared_ptr<IplImage> >  */
/* blink(currentimage, GetFurtherObject(average)); */

/* typedef TwoClassDetector<shared_ptr<IplImage> > BlinkDetector(average, blink.get()); */
