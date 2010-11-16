#pragma once
#include "utils.h"

/* class Alert { */
/*     shared_ptr<BlinkDetector> blinkdet; */
/*  public: */
/*     Alert(const shared_ptr<BlinkDetector>& blinkdet); */
/*     void alert(const string& string); */
/* }; */

enum StateEventType {
    EVENT_NOTHING, EVENT_SELECTED, EVENT_DESELECTED, EVENT_TICK,
    EVENT_BLINK, EVENT_2BLINK};


template <class ReturnType> class StateNode;

template<class ReturnType>
struct NodeResult {
    shared_ptr<StateNode<ReturnType> > nextNode;
    shared_ptr<ReturnType> result;
    NodeResult(const shared_ptr<StateNode<ReturnType> > &nextNode = 
	       shared_ptr<StateNode<ReturnType> >(),
	       const shared_ptr<ReturnType> &result = shared_ptr<ReturnType>()):
    nextNode(nextNode), result(result) {}
};

template<class ReturnType>
class StateNode {
 protected:
    int ticks;
 public:
    StateNode(): ticks(0) {}
    virtual NodeResult<ReturnType> handleEvent(StateEventType event);
    virtual ~StateNode() {};
};

template<class ReturnType>
class StateMachine: public StateNode<ReturnType>{
    shared_ptr<StateNode<ReturnType> > currentNode;
 public:
    StateMachine(const shared_ptr<StateNode<ReturnType> >& initialNode);
    virtual  NodeResult<ReturnType> handleEvent(StateEventType event);
};

template<class ReturnType>
NodeResult<ReturnType> StateNode<ReturnType>::handleEvent(StateEventType event) {
    switch(event) {
    case EVENT_SELECTED: 
    case EVENT_DESELECTED: ticks = 0; break;
    case EVENT_TICK: ticks++; break;
    default:;
    }
    return NodeResult<ReturnType>();
}

template<class ReturnType>
StateMachine<ReturnType>::StateMachine(const shared_ptr<StateNode<ReturnType> >& initialNode):
    currentNode(initialNode) 
{
}

template<class ReturnType>
NodeResult<ReturnType> StateMachine<ReturnType>::handleEvent(StateEventType event) {
    for(;;) {
	NodeResult<ReturnType> result = currentNode->handleEvent(event);
	if (result.result.get() != 0)
	    return result;
	if (result.nextNode.get() == 0) 
	    return result;
	currentNode->handleEvent(EVENT_DESELECTED);
	currentNode = result.nextNode;
	event = EVENT_SELECTED;
    }
}



class AlertWindow: public StateNode<void> {
    Gtk::Window window;
    string text;
 public:
    AlertWindow(const string &text);
    virtual ~AlertWindow();
    virtual NodeResult<void> handleEvent(StateEventType event);
};



/* class MessageNode: public StateNode { */
/*     shared_ptr<StateNode> oknode, cancelnode; */
/*     scoped_ptr<AlertWindow> alert; */
/*     string text; */
/*  public: */
/*     MessageNode(const shared_ptr<StateNode>& oknode, */
/* 		const shared_ptr<StateNode>& cancelnode, */
/* 		const string& text); */

/*     virtual shared_ptr<StateNode> handleEvent(StateEventType event); */
/* }; */
		
