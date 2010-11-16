#include "GtkStore.h"

WindowStore::WindowStore(const WindowPointer::PointerSpec& pointerspec,
			 const WindowPointer::PointerSpec& targetspec):
    pointer(pointerspec), target(targetspec)
{
}

void WindowStore::store(const TrackerOutput &output) {
    pointer.setPosition((int) output.gazepoint.x, (int) output.gazepoint.y);
    target.setPosition((int) output.target.x, (int) output.target.y);
}
