#pragma once
#include <gtkmm.h>
#include "OutputMethods.h"
#include "GraphicalPointer.h"


class WindowStore: public AbstractStore {
    WindowPointer pointer, target;
 public:
    WindowStore(const WindowPointer::PointerSpec& pointerspec,
		const WindowPointer::PointerSpec& targetspec);
    virtual void store(const TrackerOutput &output);
};
