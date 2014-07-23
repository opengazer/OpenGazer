#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
	_positionTable[0] = (int)output.gazepoint.x - 320;
	_positionTable[1] = (int)output.gazepoint.y - 240;
}

StreamStore::StreamStore(ostream &stream):
	_stream(stream)
{
}

StreamStore::~StreamStore() {}

void StreamStore::store(const TrackerOutput &output) {
	_stream << (int)output.gazepoint.x << " " << (int)output.gazepoint.y << " -> " << output.targetid << endl;
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
	ostringstream stream;
	stream << "x " << (int)output.gazepoint.x << endl << "y " << (int)output.gazepoint.y << endl;
	string str = stream.str();
	sendto(_mySocket, str.c_str(), str.size(), 0, (sockaddr *)&_destAddr, sizeof(_destAddr));
}

