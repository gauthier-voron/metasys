#include <metasys/sched/Sigaction.hxx>

#include <gtest/gtest.h>

#include <metasys/sched/ThisProcess.hxx>


using metasys::Sigaction;
using metasys::ThisProcess;


static void SimpleHandler_sighandler(int)
{
}

TEST(Sigaction, SimpleHandler)
{
	Sigaction sa = Sigaction(SimpleHandler_sighandler);
	int sig;

	EXPECT_EQ(sa.sa_handler, SimpleHandler_sighandler);
	EXPECT_EQ(sa.sa_flags, 0);

	for (sig = 1; sig < SIGRTMAX; sig++)
		EXPECT_FALSE(sa.ismasked(sig));
}

static void CompleteHandler_sighandler(int, siginfo_t *, void *)
{
}

TEST(Sigaction, CompleteHandler)
{
	Sigaction sa = Sigaction(CompleteHandler_sighandler);
	int sig;

	EXPECT_EQ(sa.sa_sigaction, CompleteHandler_sighandler);
	EXPECT_EQ(sa.sa_flags, SA_SIGINFO);

	for (sig = 1; sig < SIGRTMAX; sig++)
		EXPECT_FALSE(sa.ismasked(sig));
}

static void SimpleCtorMask_sighandler(int)
{
}

TEST(Sigaction, SimpleCtorMask)
{
	Sigaction sa = Sigaction(SimpleCtorMask_sighandler, 0,
				 Sigaction::Mask<SIGINT, SIGTERM>());
	int sig;

	EXPECT_EQ(sa.sa_handler, SimpleCtorMask_sighandler);
	EXPECT_EQ(sa.sa_flags, 0);

	for (sig = 1; sig < SIGRTMAX; sig++) {
		if ((sig == SIGINT) || (sig == SIGTERM))
			EXPECT_TRUE(sa.ismasked(sig));
		else
			EXPECT_FALSE(sa.ismasked(sig));
	}
}

static void CompleteCtorMask_sighandler(int, siginfo_t *, void *)
{
}

TEST(Sigaction, CompleteCtorMask)
{
	Sigaction sa = Sigaction(CompleteCtorMask_sighandler, 0,
				 Sigaction::Mask<SIGINT, SIGTERM>());
	int sig;

	EXPECT_EQ(sa.sa_sigaction, CompleteCtorMask_sighandler);
	EXPECT_EQ(sa.sa_flags, SA_SIGINFO);

	for (sig = 1; sig < SIGRTMAX; sig++) {
		if ((sig == SIGINT) || (sig == SIGTERM))
			EXPECT_TRUE(sa.ismasked(sig));
		else
			EXPECT_FALSE(sa.ismasked(sig));
	}
}

static volatile bool SimpleKill_delivered;
static void SimpleKill_sighandler(int)
{
	SimpleKill_delivered = true;
}

TEST(Sigaction, SimpleKill)
{
	SimpleKill_delivered = false;

	Sigaction(SimpleKill_sighandler).install(SIGINT);

	ThisProcess::kill(SIGINT);

	while (SimpleKill_delivered == false)
		;
}

static int CompleteCallComplete_sig;
static siginfo_t *CompleteCallComplete_info;
static void *CompleteCallComplete_ucontext;
static void CompleteCallComplete_sighandler(int sig, siginfo_t *info,
					    void *ucontext)
{
	CompleteCallComplete_sig = sig;
	CompleteCallComplete_info = info;
	CompleteCallComplete_ucontext = ucontext;
}

TEST(Sigaction, CompleteCallComplete)
{
	Sigaction sa = Sigaction(CompleteCallComplete_sighandler);

	CompleteCallComplete_sig = 0;
	CompleteCallComplete_info = nullptr;
	CompleteCallComplete_ucontext = nullptr;

	sa(SIGTERM, (siginfo_t *) 0x1, (void *) 0x2);

	EXPECT_EQ(CompleteCallComplete_sig, SIGTERM);
	EXPECT_EQ(CompleteCallComplete_info, (siginfo_t *) 0x1);
	EXPECT_EQ(CompleteCallComplete_ucontext, (void *) 0x2);
}

static int CompleteCallSimple_sig;
static void CompleteCallSimple_sighandler(int sig)
{
	CompleteCallSimple_sig = sig;
}

TEST(Sigaction, CompleteCallSimple)
{
	Sigaction sa = Sigaction(CompleteCallSimple_sighandler);

	CompleteCallSimple_sig = 0;

	sa(SIGTERM, (siginfo_t *) 0x1, (void *) 0x2);

	EXPECT_EQ(CompleteCallSimple_sig, SIGTERM);
}
