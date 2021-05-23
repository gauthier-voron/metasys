#include <metasys/sched/Pthread.hxx>

#include <unistd.h>

#include <cassert>
#include <string>

#include <gtest/gtest.h>


using metasys::Pthread;
using std::string;


static void *__idle_routine(void *args)
{
	volatile bool *state = reinterpret_cast<bool *> (args);

	while (*state)
		::usleep(1000);

	return NULL;
}

static pthread_t __get_idle_thread(volatile bool *state)
{
	pthread_t tid;
	int ret;

	ret = ::pthread_create(&tid, NULL, __idle_routine, (void *) state);

	assert(ret == 0);

	return tid;
}


TEST(Pthread, Unassigned)
{
	Pthread t;

	EXPECT_FALSE(t.valid());
}

TEST(Pthread, ConstructFromTid)
{
	volatile bool state = true;
	pthread_t tid = __get_idle_thread(&state);

	{
		Pthread t = Pthread(tid);

		EXPECT_TRUE(t.valid());

		t.reset();

		EXPECT_FALSE(t.valid());
	}

	state = false;
	ASSERT_EQ(::pthread_join(tid, NULL), 0);
}

TEST(Pthread, ConstructAndJoin)
{
	volatile bool state = true;
	pthread_t tid = __get_idle_thread(&state);

	{
		Pthread t = Pthread(tid);

		EXPECT_TRUE(t.valid());

		state = false;
		t.join();
	}
}

TEST(Pthread, ConstructAndTerminate)
{
	EXPECT_DEATH({
	bool state = true;
	pthread_t tid = __get_idle_thread(&state);

	{
		Pthread t = Pthread(tid);

		EXPECT_TRUE(t.valid());
	}

	FAIL();

	}, "terminate called without an active exception");
}

static volatile void *Create_args = NULL;
static void *Create_routine(void *args)
{
	Create_args = args;

	return (void *) 11;
}

TEST(Pthread, Create)
{
	void *ret, *ptr = (void *) 42;
	Pthread t;

	t.create(Create_routine, ptr);

	EXPECT_TRUE(t.valid());

	ret = t.join();

	EXPECT_EQ(Create_args, (void *) 42);
	EXPECT_EQ(ret, (void *) 11);
}

static volatile void *CreateInit_args = NULL;
static void *CreateInit_routine(void *args)
{
	CreateInit_args = args;

	return (void *) 11;
}

TEST(Pthread, CreateInit)
{
	void *ret, *ptr = (void *) 42;
	Pthread t = Pthread<>::createinit(CreateInit_routine, ptr);

	EXPECT_TRUE(t.valid());

	ret = t.join();

	EXPECT_EQ(CreateInit_args, (void *) 42);
	EXPECT_EQ(ret, (void *) 11);
}

static volatile const char *CreateTypedPtr_args = NULL;
static void CreateTypedPtr_routine(const char *args)
{
	CreateTypedPtr_args = args;
}

TEST(Pthread, CreateTypedPtr)
{
	const char *str = "Hello World!";
	Pthread<void> t;

	t.create(CreateTypedPtr_routine, str);

	EXPECT_TRUE(t.valid());

	t.join();

	EXPECT_EQ(CreateTypedPtr_args, str);
}

static volatile uint64_t CreateNonPtr_args = 0;
static void CreateNonPtr_routine(uint64_t args)
{
	CreateNonPtr_args = args;
}

TEST(Pthread, CreateNonPtr)
{
	Pthread<void> t;

	t.create(CreateNonPtr_routine, 5000000000ul);

	EXPECT_TRUE(t.valid());

	t.join();

	EXPECT_EQ(CreateNonPtr_args, 5000000000ul);
}

static volatile bool CreateCall_args = false;
static void CreateCall_routine()
{
	CreateCall_args = true;
}

TEST(Pthread, CreateCall)
{
	Pthread<void> t;

	t.create(CreateCall_routine);

	EXPECT_TRUE(t.valid());

	t.join();

	EXPECT_TRUE(CreateCall_args);
}

TEST(Pthread, CreateParamMethod)
{
	Pthread<size_t> t;
	string s = "Hello World!";
	size_t len;

	t.create<&string::length>(&s);

	EXPECT_TRUE(t.valid());

	len = t.join();

	EXPECT_EQ(len, s.length());
}

TEST(Pthread, CreateParamLambda)
{
	Pthread<bool> t;
	bool ret;

	t.create<[](int num) {
		return (num == 13);
	}>(13);

	EXPECT_TRUE(t.valid());

	ret = t.join();

	EXPECT_TRUE(ret);
}

TEST(Pthread, CreateNoParamLambda)
{
	Pthread<int> t;
	int ret;

	t.create<[]() {
		return 42;
	}>();

	EXPECT_TRUE(t.valid());

	ret = t.join();

	EXPECT_EQ(ret, 42);
}

class CreateParamVirtualMethod_Base
{
 public:
	virtual bool f()
	{
		return false;
	}
};

class CreateParamVirtualMethod_Child : public CreateParamVirtualMethod_Base
{
 public:
	bool f() override
	{
		return true;
	}
};

TEST(Pthread, CreateParamVirtualMethod)
{
	Pthread<bool> t;
	CreateParamVirtualMethod_Child child;
	bool ret;

	t.create<&CreateParamVirtualMethod_Base::f>
		(reinterpret_cast<CreateParamVirtualMethod_Base *> (&child));

	EXPECT_TRUE(t.valid());

	ret = t.join();

	EXPECT_TRUE(ret);
}

static volatile bool CreateAndCancel_flag = false;
static void CreateAndCancel_routine()
{
	CreateAndCancel_flag = true;
	while (true)
		::sleep(0);
}

TEST(Pthread, CreateAndCancel)
{
	Pthread<void> t;

	t.create(CreateAndCancel_routine);

	EXPECT_TRUE(t.valid());

	while (CreateAndCancel_flag == false)
		;

	t.cancel();

	t.join();
}

static volatile bool CreateAndCancelCatch_flag = false;
static volatile bool CreateAndCancelCatch_ret = false;
static void CreateAndCancelCatch_routine()
{
	try {
		CreateAndCancelCatch_flag = true;
		while (true)
			::sleep(0);
	} catch (...) {
		CreateAndCancelCatch_ret = true;
		throw;
	}
}

TEST(Pthread, CreateAndCancelCatch)
{
	Pthread<void> t;

	t.create(CreateAndCancelCatch_routine);

	EXPECT_TRUE(t.valid());

	while (CreateAndCancelCatch_flag == false)
		;

	t.cancel();

	t.join();

	EXPECT_TRUE(CreateAndCancelCatch_ret);
}
