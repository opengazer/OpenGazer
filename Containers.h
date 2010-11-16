#pragma once
#include "utils.h"
#include <glibmm.h>
#include <vector>

#define xforeachactive(iter,container) \
    for(typeof(container.begin()) iter = container.begin();	\
	iter != container.end(); iter++)			\
	if ((*iter)->parent == this)

template <class ParentType, class ChildType> class Container;

template <class ParentType, class ChildType> 
class  Containee {
 protected:
    void detach() { parent = 0; }
 public:
    ParentType *parent;		/* set to null to request removal */
    Containee(): parent(0) {}
    virtual ~Containee() {}
};

template <class ParentType, class ChildType>		
class Container {
    typedef shared_ptr<ChildType> ChildPtr;
    static bool isFinished(const ChildPtr &object) { 
	return !(object && object->parent);
    }
 protected:
    std::vector<ChildPtr> objects;

    void removeFinished() { 
	objects.erase(remove_if(objects.begin(), objects.end(), isFinished),
		      objects.end());
    }

 public:
    void clear() {
	xforeachactive(iter, objects)
	    (*iter)->parent = 0;
	removeFinished();
    }
	    

    static void addchild(ParentType *parent, const ChildPtr &child) {
	parent->objects.push_back(child);
	child->parent = parent;
 	parent->removeFinished(); 
    }

    virtual ~Container() {
	clear();
    }
};

template <class ParentPtr, class ChildPtr>
class ProcessContainer: public Container<ParentPtr, ChildPtr> {
 public:
    virtual void process() {
	xforeachactive(iter, this->objects)
	    (*iter)->process();
 	this->removeFinished(); 
    }
    virtual ~ProcessContainer() {};
};

