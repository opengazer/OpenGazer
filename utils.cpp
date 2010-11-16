#include "utils.h"

void releaseImage(IplImage *image) {
//     cout << "deleting shared image" << endl;
    cvReleaseImage(&image);
}

shared_ptr<IplImage> createImage(const CvSize &size, int depth, int channels) {
    return shared_ptr<IplImage>(cvCreateImage(size, depth, channels),
				releaseImage);
}

namespace boost {
    template<>
    void checked_delete(IplImage *image) {
// 	cout << "deleting scoped image" << endl;
	if (image)
	    cvReleaseImage(&image);
    }
}
