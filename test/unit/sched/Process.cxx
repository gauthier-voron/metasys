#include <metasys/sched/Process.hxx>

#include <unistd.h>

#include <gtest/gtest.h>


using metasys::Process;


static pid_t __fork_short()
{
	pid_t ret = ::fork();

	assert(ret >= 0);

	if (ret == 0)
		::exit(0);

	return ret;
}

static inline void __get_pipe(int fds[2])
{
	assert(::pipe(fds) == 0);
}


TEST(Process, Unassigned)
{
	Process p;

	EXPECT_FALSE(p.valid());
}

TEST(Process, ConstructFromPid)
{
	pid_t pid = __fork_short();

	{
		Process p = Process(pid);

		EXPECT_TRUE(p.valid());

		p.reset();

		EXPECT_FALSE(p.valid());
	}

	ASSERT_EQ(::waitpid(pid, NULL, 0), pid);
}

TEST(Process, ForkInit)
{
	pid_t self = ::getpid();
	pid_t pid;

	{
		Process p = Process<>::forkinit();

		if (::getpid() != self)
			::exit(0);

		EXPECT_TRUE(p.valid());

		pid = p.pid();

		EXPECT_GT(pid, 0);
	}

	ASSERT_EQ(::waitpid(pid, NULL, 0), pid);
}
