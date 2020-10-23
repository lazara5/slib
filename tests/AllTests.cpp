#include "CppUTest/CommandLineTestRunner.h"

#include "Global.h"

int test_argc;
char** test_argv;

int main(int argc, char** argv) {
	MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
	test_argc = argc;
	test_argv = argv;
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
