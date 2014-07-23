#pragma once

#include <gtkmm.h>

#include "OutputMethods.h"
#include "GraphicalPointer.h"

class WindowStore: public AbstractStore {
public:
	WindowStore(const WindowPointer::PointerSpec &pointerSpec, const WindowPointer::PointerSpec &pointerSpecLeft, const WindowPointer::PointerSpec &targetSpec);
	virtual void store(const TrackerOutput &output);

private:
	WindowPointer _pointer;
	WindowPointer _pointerLeft;
	WindowPointer _target;
};
