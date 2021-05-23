#include <metasys/sched/Process.hxx>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>

#include <asmcmp.hxx>


using metasys::Process;
using metasys::ProcessBehavior;


Model(ForkUnsafe)
{
	pid_t pid = ::fork();

	if (pid == 0) {
		printf("child\n");
		::exit(0);
	}

	printf("parent\n");
}
Test(ForkUnsafe)
{
	Process proc = Process<>::forkinit([](auto){});

	if (proc.pid() == 0) {
		printf("child\n");
		::exit(0);
	}

	printf("parent\n");
}


Model(ForkSafe)
{
	pid_t pid = ::fork();

	if (pid < 0) [[unlikely]]
		return;

	if (pid == 0) {
		printf("child\n");
		::exit(0);
	}

	printf("parent\n");
}
Test(ForkSafe)
{
	Process proc = Process<>::forkinit();

	if (proc.pid() == 0) {
		printf("child\n");
		::exit(0);
	}

	printf("parent\n");
}


Model(ForkWaitUnsafe)
{
	pid_t pid = ::fork();

	if (pid == 0) {
		printf("child\n");
		::exit(0);
	}

	::waitpid(pid, NULL, 0);

	printf("parent\n");
}
Test(ForkWaitUnsafe)
{
	Process proc = Process<>::forkinit([](auto){});

	if (proc.pid() == 0) {
		printf("child\n");
		::exit(0);
	}

	proc.wait([](auto){});

	printf("parent\n");
}


// static void __routine_ForkToUnsafe(int a, int b)
// {
// 	printf("%d\n", a + b);
// }

// Model(ForkToUnsafe)
// {
// 	pid_t pid = ::fork();

// 	if (pid == 0) {
// 		__routine_ForkToUnsafe(13, 16);
// 		::exit(0);
// 	}

// 	::waitpid(pid, NULL, 0);
// }
// Test(ForkToUnsafe)
// {
// 	Process<>::forktoinit(__routine_ForkToUnsafe, 13, 14, [](auto){})
// 		.wait([](auto){});
// }
