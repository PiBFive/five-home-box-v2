#include <iostream>
#include "Manager.h"
#include "Options.h"
#include "Driver.h"
#include <thread>
#include <fstream>
#include "Five.h"

using namespace OpenZWave;
using namespace Five;
using namespace std;

bool g_menuLocked{true};
bool g_checkLocked(1);

int main(int argc, char const *argv[]) {
	fstream my_file;
	string response;

	Five::startedAt = getCurrentDatetime();
	
	response = "3";

	// cout << ">>──── LOG LEVEL ────<<\n\n"
	// 	 << "     0. NONE\n"
	// 	 << "     1. WARNING\n"
	// 	 << "     2. INFO\n"
	// 	 << "     3. DEBUG\n\n"
	// 	 << "Select the log level (0, 1, 2, 3): ";
	// cin >> response;

	switch (stoi(response))
	{
	case 0:
		LEVEL = logLevel::NONE;
		break;
	case 1:
		LEVEL = logLevel::WARNING;
		break;
	case 2:
		LEVEL = logLevel::INFO;
		break;
	case 3:
		LEVEL = logLevel::DEBUG;
		break;
	default:
		LEVEL = logLevel::NONE;
		break;
	}

	pthread_mutexattr_t mutexattr;

	// Initialize the mutex ATTRIBUTES with the default value.
	pthread_mutexattr_init(&mutexattr);

	/* PTHREAD_MUTEX_NORMAL: it does not detect dead lock.
	   - If you want lock a locked mutex, you will get a deadlock mutex.
	   - If you want unlock an unlocked mutex, you will get an undefined behavior.

	   PTHREAD_MUTEX_ERRORCHECK: it returns an error if you you want lock a locked mutex or unlock an unlocked mutex.

	   PTHREAD_MUTEX_RECURSIVE:
	   - If you want lock a locked mutex, it stays at locked state an returns a success.
	   - If you want unlock an unlocked mutex, it stays at unlocked state an returns a success.

	   PTHREAD_MUTEX_DEFAULT: if you lock a locked mutex or unlock an unlocked mutex, you will get an undefined behavior.
	*/
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

	/* Initialize the mutex referenced by mutex with attributes specified by attr.
	   If the attribute is NULL, the default mutex attributes are used.
	   Upon initialization is successful, the mutex becomes INITIALIZED and UNLOCKED.

	   /!\ Initialization the same mutex twice will get an undefined behavior. /!\
	*/
	pthread_mutex_init(&g_criticalSection, &mutexattr);

	// You only can use the mutex ATTR one time, it does not affect any other mutexes and you should detroy it to uninitialize it.
	pthread_mutexattr_destroy(&mutexattr);

	pthread_mutex_lock(&initMutex);

	Options::Create(CONFIG_PATH, CACHE_PATH, "");
	Options::Get()->Lock();
	Manager::Create();
	Manager::Get()->AddWatcher(onNotification, NULL);

	const char *cmd01 = "rm cpp/examples/cache/.config";
	const char *cmd02 = "ls -l /dev/ttyACM* | awk {'print($10)'} > cpp/examples/cache/.config";

	cout << "[bash " << system(cmd01) << "] " << cmd01 << endl;
	cout << "[bash " << system(cmd02) << "] " << cmd02 << endl;

	my_file.open("cpp/examples/cache/.config");
	
	if (my_file.is_open()) {
		while(my_file.good()) {
			my_file >> DRIVER_PATH;
		}
	}

	my_file.close();

	Manager::Get()->AddDriver(DRIVER_PATH);

	thread t3(Five::statusObserver, nodes);
	thread t4(server, ZWAVE_PORT);
	thread t2(Five::CheckFailedNode, FAILED_NODE_PATH);
	
	t3.detach();
	t4.detach();
	t2.detach();
	
	pthread_cond_wait(&initCond, &initMutex);
	pthread_mutex_unlock(&g_criticalSection);
	
	return EXIT_SUCCESS;
}
