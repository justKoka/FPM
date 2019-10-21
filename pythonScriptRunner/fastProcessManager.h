#pragma once
#include <python2.7/Python.h>
#include <cstdio>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <unordered_map>

enum pstatus {
	WAIT,
	WORK
};

struct pinfo {
	pid_t pid;
	pstatus status;
};

void sigHandler(int, siginfo_t *, void *);

class fastProcessManager
{
public:
	fastProcessManager(uint32_t buffsize);
	~fastProcessManager();
	void run(int numpr);
	void listen();
private:
	bool ptrigger(const char *ev);
	void shutdown();
	void initProcess();
	void resetProcess(int fd);
	std::unordered_map<int, pinfo> pmap;
	int nfds;
	std::vector<pollfd> fdset;
	const uint32_t MAX_BUFFER_SIZE;
};