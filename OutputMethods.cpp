#include "OutputMethods.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

AbstractStore::~AbstractStore() {
}

MmapStore::MmapStore(const char *filename) {
    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    write(fd, &fd, sizeof(fd));
    write(fd, &fd, sizeof(fd));
    if (fd < 0) {perror("open");return;}
    positiontable = (int*) mmap(0, getpagesize(), PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, 0);
}

void MmapStore::store(const TrackerOutput& output) {
    positiontable[0] = (int) output.gazepoint.x - 320;
    positiontable[1] = (int) output.gazepoint.y - 240;
}

MmapStore::~MmapStore() {
    munmap(positiontable, getpagesize());
    close(fd);
}
 
StreamStore::StreamStore(ostream &stream): stream(stream) {
}

StreamStore::~StreamStore() {
}

void StreamStore::store(const TrackerOutput& output) {
    stream << (int) output.gazepoint.x << " " 
	   << (int) output.gazepoint.y << " -> "
	   << output.targetid << endl;
    stream.flush();
}

SocketStore::SocketStore(int port) {
    mysocket = socket(PF_INET, SOCK_DGRAM, 0);
	
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(port);
    destaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
}

void SocketStore::store(const TrackerOutput& output) {
    ostringstream stream;
    stream << "x " << (int) output.gazepoint.x << endl
	   << "y " << (int) output.gazepoint.y << endl;
    string str = stream.str();
    sendto(mysocket, str.c_str(), str.size(), 0, 
	   (sockaddr*)&destaddr, sizeof(destaddr));
}

SocketStore::~SocketStore(void) {
    close(mysocket);
}

