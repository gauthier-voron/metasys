#include <metasys/sched/Sigaction.hxx>

#include <cstdio>

#include <asmcmp.hxx>


using metasys::Sigaction;


static void handler(int)
{
}


Model(InstallSafe)
{
	struct sigaction sa;

	sa.sa_handler = handler;
	sa.sa_flags = 0;
	::sigemptyset(&sa.sa_mask);

	::sigaction(SIGINT, &sa, NULL);
}
Test(InstallSafe)
{
	Sigaction(handler).install(SIGINT);
}


Model(SaveSafe)
{
	struct sigaction sa;

	::sigaction(SIGINT, NULL, &sa);

	printf("%d\n", sa.sa_flags);
}
Test(SaveSafe)
{
	Sigaction sa = Sigaction::saveinit(SIGINT);

	printf("%d\n", sa.sa_flags);
}
