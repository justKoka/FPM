#include <python2.7/Python.h>
#include <cstdio>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

enum pstatus {
	WAIT,
	WORK
};

struct pinfo {
	int sfd;
	pid_t pid;
	pstatus status;
};

void sigHandler(int, siginfo_t *, void *);
bool ptrigger(std::vector<pinfo> *, const char *);

int main()
{	
	char file[] = "/home/koka/Documents/test.py";
	int pnum = 1;
	std::vector<pinfo> pvec;
	pvec.reserve(pnum);
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sigHandler;
	int flags = SA_SIGINFO | SA_NOCLDWAIT;
	act.sa_flags = flags;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	act.sa_mask = set;
	sigaction(SIGCHLD, &act, NULL);
	for (int i = 0; i < pnum; ++i) {
		int fds[2];
		socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
		auto pid = fork();
		if (pid == 0) {
			while (true) {
				pollfd fdset;
				fdset.fd = fds[1];
				fdset.events = POLLIN;
				poll(&fdset, 1, -1);
				const uint32_t length = 512;
				char filename[length];
				int readlen = read(fds[1], filename, length);
				Py_Initialize();
				if (!Py_IsInitialized())
				{
					printf("init fail\n");
					exit(1);
				}
				PyRun_SimpleString("import sys\nclass StdoutCatcher:\n	def __init__(self) :\n		self.data = ''\n	def write(self, stuff) :\n		self.data = self.data + stuff\ncatcher = StdoutCatcher()\nsys.stdout = catcher");
				PyObject* m = PyImport_AddModule("__main__");
				FILE *fd = fopen(filename, "r");
				if (fd == NULL) {
					printf("file opening failed\n");
					exit(2);
				}
				if (PyRun_SimpleFile(fd, file))
				{
					printf("script running failed\n");
					exit(3);
				}
				PyObject* catcher = PyObject_GetAttrString(m, "catcher");
				PyObject* output = PyObject_GetAttrString(catcher, "data");
				char *buffer = PyString_AsString(output);
				printf("data in child process was: %s\n", buffer);
				write(fds[1], buffer, strlen(buffer));
				fclose(fd);
				Py_XDECREF(catcher);
				Py_XDECREF(output);
				Py_Finalize();
			}
		}
		else
		{
			pinfo pmember;
			pmember.pid = pid;
			pmember.sfd = fds[0];
			pmember.status = WAIT;
			pvec.push_back(pmember);
			ptrigger(&pvec, file);
			pollfd fdset;
			fdset.fd = pvec[i].sfd;
			fdset.events = POLLIN;
			poll(&fdset, 1, -1);
			char BUF[512];
			int l = read(pvec[i].sfd, BUF, 512);
			BUF[l] = '\0';
			printf("data in parent process was: %s\n", BUF);
			pvec[i].status = WAIT;
		}
	}
	return 0;
}

void sigHandler(int signum, siginfo_t *sinfo, void *ucontext)
{
	printf("signal %d arrived with exit value of %d\n", signum, sinfo->si_status);
}

bool ptrigger(std::vector<pinfo> *pv, const char* ev) {
	for (auto it = (*pv).begin(); it != (*pv).end(); ++it)
		if ((*it).status == WAIT) {
			write((*it).sfd, ev, strlen(ev));
			(*it).status = WORK;
			return true;
		}
	return false;
}