#include "Alert.h"

AlertWindow::AlertWindow(const string& text): text(text) {
    window.set_title(text);
    window.show();
}

AlertWindow::~AlertWindow() {}

NodeResult<void> AlertWindow::handleEvent(StateEventType event) {
    StateNode<void>::handleEvent(event);
    if (ticks >= 50)
	return NodeResult<void>(shared_ptr<AlertWindow>
			  (new AlertWindow(text + " x")));
    else
	return NodeResult<void>();
}


// MessageNode::MessageNode(const shared_ptr<StateNode>& oknode,
// 			 const shared_ptr<StateNode>& cancelnode,
// 			 const string& text):
//     oknode(oknode), cancelnode(cancelnode), text(text)
// {
// }

// shared_ptr<StateNode> MessageNode::handleEvent(StateEventType event) {
//     switch (event) {
//     case EVENT_2BLINK: return oknode;
//     case EVENT_TIMEOUT: return cancelnode;
//     case EVENT_DESELECTED: 
// 	alert = 0;
// 	return StateNode::handleEvent(event);
//     case EVENT_SELECTED: 
// 	alert = scoped_ptr<AlertWindow>(new AlertWindow(text));
// 	return StateNode::handleEvent(event);
//     default: return StateNode::handleEvent(event);
//     }
// }


