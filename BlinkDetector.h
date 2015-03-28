#pragma once

#include <boost/scoped_ptr.hpp>

struct StateNode {
	int minDuration;
	int maxDuration;
	double threshold;
	double finishThreshold;

	StateNode(int minDuration, int maxDuration, double threshold, double finishThreshold);
	bool isSatisfied(double value) const;
	bool isFinished(double value) const;
};

class LinearStateSystem {
public:
	LinearStateSystem(const std::vector<StateNode> &states);
	void updateState(double value);
	void setState(int state);
	int getState();
	bool isFinalState();

private:
	const std::vector<StateNode> _states;
	int _currentState;
	int _duration;
};

class LambdaAccumulator {
 public:
	LambdaAccumulator(double lambda, double initial);
	void update(double value);
	double getValue();

private:
	const double _lambda;
	double _current;
};

class BlinkDetector {
 public:
	BlinkDetector();
	void update(const boost::scoped_ptr<cv::Mat> &image);
	int getState();

private:
	boost::scoped_ptr<cv::Mat> _averageEye;
	LambdaAccumulator _accumulator;
	LinearStateSystem _states;
	bool _initialized;

	static std::vector<StateNode> constructStates();
};

//template <class T> class TwoClassDetector {
//public:
//	TwoClassDetector(const T& class0, const T& class1):
//		_class0(class0),
//		_class1(class1)
//	{
//	}

//	double classify(const T& object) {
//		double dist0 = distance(_class0, object);
//		double dist1 = distance(_class1, object);
//		return dist0 / (dist0 + dist1);
//	};

//private:
//	const T _class0;
//	const T _class1;
//};

//template <class T, class Func> class Accumulator {
//public:
//	Accumulator(const T& initial, const Func& func):
//		_accumulator(initial),
//		_func(func)
//	{
//	};

//	void update(const T& object) {
//		_accumulator = func(_accumulator, object);
//	}

//	T get() {
//		return _accumulator;
//	}

//private:
//	T _accumulator;
//	Func _func;
//};

//template <T> class GetFurtherObject {
//public:
//	T operator()(const T& one, const T& two) {
//		return distance(_referenceObject, one) > distance(_referenceObject, two) ? one : two;
//	}

//private:
//	T _referenceObject;
//};

//Accumulator<shared_ptr<IplImage> > blink(currentimage, GetFurtherObject(average));

//typedef TwoClassDetector<shared_ptr<IplImage> > BlinkDetector(average, blink.get());
