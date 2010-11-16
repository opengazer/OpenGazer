#pragma once
#include "utils.h"
#include "GazeTracker.h"
#include <netinet/in.h>

class AbstractStore {
public:
    virtual void store(const TrackerOutput& output) = 0;
    virtual ~AbstractStore();
};

class MmapStore: public AbstractStore {
    int fd;
    int *positiontable;
public:
    MmapStore(const char *filename="/tmp/gaze-mouse");
    virtual void store(const TrackerOutput& output);
    virtual ~MmapStore();
};
 
class StreamStore: public AbstractStore {
    ostream &stream;
public:
    StreamStore(ostream &stream);
    virtual void store(const TrackerOutput& output);
    virtual ~StreamStore();
};

class SocketStore: public AbstractStore {
    int mysocket;
    struct sockaddr_in destaddr;
public:
    SocketStore(int port=20320);
    virtual void store(const TrackerOutput& output);
    virtual ~SocketStore();
};
