#include "fastProcessManager.h"

fastProcessManager::fastProcessManager(uint32_t buffsize) : MAX_BUFFER_SIZE(buffsize)
{	
}

fastProcessManager::~fastProcessManager()
{
	shutdown();
}

bool fastProcessManager::ptrigger(const char *ev)
{
	for (auto it = pmap.begin(); it != pmap.end(); ++it)
		if ((*it).second.status == WAIT) {
			write((*it).first, ev, strlen(ev));
			(*it).second.status = WORK;
			return true;
		}
	return false;
}

void fastProcessManager::shutdown()
{
	for (auto it = pmap.begin(); it != pmap.end(); ) {
		auto tmpit = it;
		++it;
		resetProcess((*tmpit).first);
	}
}

void fastProcessManager::initProcess()
{
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	auto pid = fork();
	if (pid == 0) {
		while (true) {
			pollfd fdset;
			fdset.fd = fds[1];
			fdset.events = POLLIN;
			poll(&fdset, 1, -1);
			char inbuff[MAX_BUFFER_SIZE];
			int readlen = read(fds[1], inbuff, MAX_BUFFER_SIZE);
			inbuff[readlen] = '\0';
			if (strcmp(inbuff, "kill") == 0) {
				printf("child process terminated by parent\n");
				write(fds[1], "exit 0", 6);
				close(fds[1]);
				exit(0);
			}
			Py_Initialize();
			if (!Py_IsInitialized())
			{
				printf("init fail\n");
				write(fds[1], "exit 1", 6);
				close(fds[1]);
				exit(1);
			}
			PyRun_SimpleString("import sys\nclass StdoutCatcher:\n	def __init__(self) :\n		self.data = ''\n	def write(self, stuff) :\n		self.data = self.data + stuff\ncatcher = StdoutCatcher()\nsys.stdout = catcher");
			PyObject* m = PyImport_AddModule("__main__");
			FILE *fd = fopen(inbuff, "r");
			if (fd == NULL) {
				printf("file opening failed\n");
				write(fds[1], "exit 2", 6);
				close(fds[1]);
				exit(2);
			}
			if (PyRun_SimpleFile(fd, inbuff))
			{
				printf("script running failed\n");
				write(fds[1], "exit 3", 6);
				close(fds[1]);
				exit(3);
			}
			PyObject* catcher = PyObject_GetAttrString(m, "catcher");
			PyObject* output = PyObject_GetAttrString(catcher, "data");
			char *buffer = strdup(PyBytes_AS_STRING(PyUnicode_AsEncodedString(output, "utf-8", "ERROR")));
			if (buffer != NULL)
				printf("data in child process was: %s\n", buffer);
			else
				printf("error in data fetching from python script");
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
		pmember.status = WAIT;
		pmap[fds[0]] = pmember;
		pollfd tmpfdset;
		tmpfdset.fd = fds[0];
		tmpfdset.events = POLLIN;
		tmpfdset.revents = 0;
		fdset.push_back(tmpfdset);
	}
}

void fastProcessManager::resetProcess(int fd)
{
	write(fd, "kill", 4);
	pmap.erase(fd);
	//for (auto it = fdset.begin(); it != fdset.end(); ++it)
	//	if ((*it).fd == fd)
	//	{
	//		fdset.erase(it);
	//		break;
	//	}
}

void fastProcessManager::run(int pnum)
{
	pmap.reserve(pnum);
	nfds = pnum;
	//struct sigaction act;
	//memset(&act, 0, sizeof(act));
	//act.sa_sigaction = sigHandler;
	//int flags = SA_SIGINFO | SA_NOCLDWAIT;
	//act.sa_flags = flags;
	//sigset_t set;
	//sigemptyset(&set);
	//sigaddset(&set, SIGCHLD);
	//act.sa_mask = set;
	//sigaction(SIGCHLD, &act, NULL);
	for (int i = 0; i < pnum; ++i) {
		initProcess();
	}
}

void fastProcessManager::listen()
{
	char file[] = "/home/koka/Documents/test.py";
	//for test
	ptrigger(file);
	ptrigger(file);
	ptrigger(file);
	while (true) {
		int nTriggeredFd = poll(fdset.data(), nfds, -1);
		for (auto it = fdset.begin(); it != fdset.end() && nTriggeredFd > 0;) {
			{
				if ((*it).revents != 0)
					if ((*it).revents == POLLIN)
					{
						char buffer[MAX_BUFFER_SIZE];
						int len = read((*it).fd, buffer, MAX_BUFFER_SIZE);
						buffer[len] = '\0';
						if (strncmp(buffer, "exit", 4) == 0)
						{
							int exitStat = atoi(&buffer[5]);
							close((*it).fd);
							if (exitStat != 0)
								pmap.erase((*it).fd);
							auto tmpit = it;
							if (it+1 != fdset.end())
								++it;
							fdset.erase(tmpit);
							--nTriggeredFd;
							continue;
						}
						printf("data in parent process was: %s\n", buffer);
						--nTriggeredFd;
						pmap[(*it).fd].status = WAIT;
						(*it).revents = 0;
					}
					else
						printf("error, fd %d", (*it).fd);
				if (it + 1 != fdset.end())
					++it;
			}

			}
		//for test
		/*shutdown();*/
	}
}

void sigHandler(int signum, siginfo_t *sinfo, void *ucontext)
{
	printf("signal %d arrived with exit value of %d\n", signum, sinfo->si_status);
}
