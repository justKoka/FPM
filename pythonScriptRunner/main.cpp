#include "fastProcessManager.h"

int main()
{	
	fastProcessManager fpm(1024);
	fpm.run(3);
	fpm.listen();
	return 0;
}