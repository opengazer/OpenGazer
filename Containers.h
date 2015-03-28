#pragma once

#include <boost/shared_ptr.hpp>

#define xForEachActive(iter, container) \
	for(typeof(container.begin())iter = container.begin(); iter != container.end(); iter++)	\
		if ((*iter)->parent == this)

template <class ParentType, class ChildType> class Containee {
public:
	ParentType *parent;		/* set to null to request removal */

	Containee(): parent(0) {}
	virtual ~Containee() {}

protected:
	void detach() {
		parent = 0;
	}
};

template <class ParentType, class ChildType> class BaseContainer {

	typedef boost::shared_ptr<ChildType> ChildPtr;

public:
	void clear() {
		xForEachActive(iter, objects) {
			(*iter)->parent = 0;
		}
		removeFinished();
	}

	static void addChild(ParentType *parent, const ChildPtr &child) {
		parent->objects.push_back(child);
		child->parent = parent;
		parent->removeFinished();
	}

	virtual ~BaseContainer() {
		clear();
	}

protected:
	std::vector<ChildPtr> objects;

	void removeFinished() {
		objects.erase(remove_if(objects.begin(), objects.end(), isFinished), objects.end());
	}

private:
	static bool isFinished(const ChildPtr &object) {
		return !(object && object->parent);
	}
};

template <class ParentPtr, class ChildPtr> class ProcessContainer: public BaseContainer<ParentPtr, ChildPtr> {
public:
	virtual ~ProcessContainer() {};

	virtual void process() {
		xForEachActive(iter, this->objects) {
			(*iter)->process();
		}
		this->removeFinished();
	}
};

