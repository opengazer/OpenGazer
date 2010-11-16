#include "BlinkDetector.h"
#include <iostream>
#include "EyeExtractor.h"
#include "utils.h"

StateNode::StateNode(int minduration, int maxduration, double threshold):
    minduration(minduration), maxduration(maxduration), threshold(threshold) 
{
}

bool StateNode::isSatisfied(double value) const {
    if (threshold > 0)
	return value >= threshold;
    else
	return value <= -threshold;
}

LinearStateSystem::LinearStateSystem(const vector<StateNode> &states): 
    states(states), currentState(0), duration(0)
{
    assert(!states.empty());
}

void LinearStateSystem::updateState(double value) {
    cout << "update state: " << value << endl;
    duration++;
    if (currentState > 0 &&
	((duration < states[currentState].minduration &&
	  !states[currentState].isSatisfied(value)) || 
	 duration > states[currentState].maxduration))
	setState(0);

    if (!isFinalState() && states[currentState+1].isSatisfied(value))
	setState(currentState+1);
}

void LinearStateSystem::setState(int state) {
    currentState = state;
    duration = 0;
}

int LinearStateSystem::getState() {
    return currentState;
}

bool LinearStateSystem::isFinalState() {
    return currentState == (int)states.size() - 1;
}

LambdaAccumulator::LambdaAccumulator(double lambda, double initial):
    lambda(lambda), current(initial)
{
}

void LambdaAccumulator::update(double value) {
    current = (1-lambda)*current + lambda*value;
}

double LambdaAccumulator::getValue() {
    return current;
}

BlinkDetector::BlinkDetector():
    averageEye(cvCreateImage(EyeExtractor::eyesize, IPL_DEPTH_32F, 1)),
    acc(0.1, 1000.0), 
    states(constructStates())
{
}

vector<StateNode> BlinkDetector::constructStates() {
    vector<StateNode> states;
    states.push_back(StateNode(0, 0, +0.00)); 
    states.push_back(StateNode(1, 8, +1.50)); 
    states.push_back(StateNode(1, 8, -1.20)); // blink
    states.push_back(StateNode(1, 8, +1.50)); 
    states.push_back(StateNode(1, 8, -1.20)); // double blink
    return states;
}

void BlinkDetector::update(const scoped_ptr<IplImage> &eyefloat) {
    double distance = cvNorm(eyefloat.get(), averageEye.get(), CV_L2);
    acc.update(distance);
    cout << "update distance" << distance << " -> " << acc.getValue() << endl;
    states.updateState(distance / acc.getValue());
    cvRunningAvg(eyefloat.get(), averageEye.get(), 0.05);
}

int BlinkDetector::getState() {
    return states.getState();
}
