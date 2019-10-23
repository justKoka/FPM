#include "fastProcessManager.h"

int main()
{	
	fastProcessManager fpm(1024);
	fpm.run(3);
	fpm.listen();
	return 0;
}

//int main() {
//	char inbuff[512] = "/home/koka/Documents/test.py";
//	const int readlen = 29;
//	inbuff[readlen] = '\0';
//	std::unique_ptr<char[]> input(new char[readlen + 46]);
//	sprintf(input.get(), "oldstdin = sys.stdin\nsys.stdin=StringIO('%s')", inbuff);
//	Py_Initialize();
//	if (!Py_IsInitialized())
//	{
//		printf("init fail\n");
//	}
//	int ret = PyRun_SimpleString("import sys\nfrom io import StringIO\nclass StdoutCatcher:\n	def __init__(self) :\n		self.data = ''\n	def write(self, stuff) :\n		self.data = self.data + stuff\n	def flush(self):		self.data = self.data\ncatcher = StdoutCatcher()\nsys.stdout = catcher");
//	ret = PyRun_SimpleString(input.get());
//	FILE *fd = fopen(inbuff, "r");
//	if (fd == NULL) {
//		printf("file opening failed\n");
//	}
//	PyObject* m = PyImport_AddModule("__main__");
//	if (PyRun_SimpleFile(fd, "name"))
//	{
//		printf("script running failed\n");
//	}
//	PyObject* catcher = PyObject_GetAttrString(m, "catcher");
//	PyObject* output = PyObject_GetAttrString(catcher, "data");
//	char *buffer = strdup(PyBytes_AS_STRING(PyUnicode_AsEncodedString(output, "utf-8", "ERROR")));
//	if (buffer != NULL)
//		printf("data in child process was: %s\n", buffer);
//	else
//		printf("error in data fetching from python script");
//	fclose(fd);
//	Py_XDECREF(catcher);
//	Py_XDECREF(output);
//	Py_Finalize();
//}