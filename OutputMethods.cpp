#include <arpa/inet.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "OutputMethods.h"

AbstractStore::~AbstractStore() {}

MmapStore::MmapStore(const char *filename) {
	_fileDescriptor = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	write(_fileDescriptor, &_fileDescriptor, sizeof(_fileDescriptor));
	write(_fileDescriptor, &_fileDescriptor, sizeof(_fileDescriptor));
	if (_fileDescriptor < 0) {
		perror("open");
		return;
	}

	_positionTable = (int *)mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, _fileDescriptor, 0);
}

MmapStore::~MmapStore() {
	munmap(_positionTable, getpagesize());
	close(_fileDescriptor);
}

void MmapStore::store(const TrackerOutput &output) {
	_positionTable[0] = (int)output.gazePoint.x - 320;
	_positionTable[1] = (int)output.gazePoint.y - 240;
}

StreamStore::StreamStore(std::ostream &stream):
	_stream(stream)
{
}

StreamStore::~StreamStore() {}

void StreamStore::store(const TrackerOutput &output) {
	_stream << (int)output.gazePoint.x << " " << (int)output.gazePoint.y << " -> " << output.targetId << std::endl;
	_stream.flush();
}

SocketStore::SocketStore(int port) {
	_mySocket = socket(PF_INET, SOCK_DGRAM, 0);
	_destAddr.sin_family = AF_INET;
	_destAddr.sin_port = htons(port);
	_destAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
}

SocketStore::~SocketStore(void) {
	close(_mySocket);
}

void SocketStore::store(const TrackerOutput &output) {
	std::ostringstream stream;
	stream << "x " << (int)output.gazePoint.x << std::endl << "y " << (int)output.gazePoint.y << std::endl;
	std::string str = stream.str();
	sendto(_mySocket, str.c_str(), str.size(), 0, (sockaddr *)&_destAddr, sizeof(_destAddr));
}

WindowStore::WindowStore(const WindowPointer::PointerSpec &pointerSpec, const WindowPointer::PointerSpec &pointerSpecLeft, const WindowPointer::PointerSpec &targetSpec):
	_pointer(pointerSpec),
	_pointerLeft(pointerSpecLeft),
	_target(targetSpec)
{
}

void WindowStore::store(const TrackerOutput &output) {
	int numMonitors = Gdk::Screen::get_default()->get_n_monitors();

	if (numMonitors == 1) {
		_pointer.setPosition((int)output.gazePoint.x, (int)output.gazePoint.y);
		_pointerLeft.setPosition((int)output.gazePointLeft.x, (int)output.gazePointLeft.y);
		_target.setPosition((int)output.target.x, (int)output.target.y);

		//_pointer.setPosition((int)output.nnGazePoint.x, (int)output.nnGazePoint.y);
		//_pointerLeft.setPosition((int)output.nnGazePointLeft.x, (int)output.nnGazePointLeft.y);
		//_target.setPosition(0, 0);
	} else {
		Point gazePoint1, gazePoint2, targetPoint;

		// Show Gaussian process outputs
		Utils::mapToFirstMonitorCoordinates(output.gazePoint, gazePoint1);
		Utils::mapToFirstMonitorCoordinates(output.gazePointLeft, gazePoint2);

		// Show neural network results
		//Utils::mapToFirstMonitorCoordinates(output.nnGazePoint, gazePoint1);
		//Utils::mapToFirstMonitorCoordinates(output.nnGazePointLeft, gazePoint2);

		Utils::mapToFirstMonitorCoordinates(output.target, targetPoint);

		_pointer.setPosition((int)gazePoint1.x, (int)gazePoint1.y);
		_pointerLeft.setPosition((int)gazePoint2.x, (int)gazePoint2.y);
		_target.setPosition((int)targetPoint.x, (int)targetPoint.y);
	}
}

