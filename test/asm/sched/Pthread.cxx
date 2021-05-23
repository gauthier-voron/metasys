#include <metasys/sched/Pthread.hxx>

#include <pthread.h>

#include <asmcmp.hxx>


using metasys::Pthread;
using metasys::PthreadBehavior;


static void *__routine_CreateNopFuncSafe(void *)
{
	return NULL;
}

Model(CreateNopFuncSafe)
{
	Overhead(pthread_t tid, pthread_t tid = {});
	int ret;

	ret = pthread_create(&tid, NULL, __routine_CreateNopFuncSafe,NULL);

	if (ret != 0) [[unlikely]]
		return;

	asm volatile ("nop");
}
Test(CreateNopFuncSafe)
{
	Pthread<void *, PthreadBehavior::Nothing> thread;

	thread.create(__routine_CreateNopFuncSafe, nullptr);

	asm volatile ("nop");
}


Model(CreateinitNopFuncSafe)
{
	pthread_t tid;
	int ret;

	ret = pthread_create(&tid, NULL, __routine_CreateNopFuncSafe,NULL);

	if (ret != 0) [[unlikely]]
		return;

	asm volatile ("nop");
}
Test(CreateinitNopFuncSafe)
{
	Pthread<void *, PthreadBehavior::Nothing> thread =
		Pthread<void *, PthreadBehavior::Nothing>::createinit
		(__routine_CreateNopFuncSafe, nullptr);

	asm volatile ("nop");
}


Model(CreateNopFuncUnsafe)
{
	Overhead(pthread_t tid, pthread_t tid = {});

	pthread_create(&tid, NULL, __routine_CreateNopFuncSafe,NULL);
}
Test(CreateNopFuncUnsafe)
{
	Pthread<void *, PthreadBehavior::Nothing> thread;

	thread.create(__routine_CreateNopFuncSafe, nullptr, [](auto){});
}


Model(JoinVoidPtr)
{
	pthread_t tid = 0;
	void *val;

	pthread_join(tid, &val);

	asm volatile ("nop" : : "r" (val));
}
Test(JoinVoidPtr)
{
	Pthread thread;
	void *val;

	val = thread.join();

	asm volatile ("nop" : : "r" (val));
}


Model(JoinIgnoreVoidPtr)
{
	pthread_t tid = 0;

	pthread_join(tid, NULL);
}
Test(JoinIgnoreVoidPtr)
{
	Pthread<void> thread;

	thread.join();
}


Model(Detach)
{
	pthread_t tid = 0;

	pthread_detach(tid);
}
Test(Detach)
{
	Pthread thread;

	thread.detach();
}


static void *__routine_CreateNopFuncJoinSafe(void *)
{
	return NULL;
}

Model(CreateNopFuncJoinSafe)
{
	Overhead(pthread_t tid, pthread_t tid = {});
	void *val;
	int ret;

	ret = pthread_create(&tid, NULL, __routine_CreateNopFuncJoinSafe,NULL);

	if (ret != 0) [[unlikely]]
		return;

	pthread_join(tid, &val);

	asm volatile ("nop" : : "r" (val));
}
Test(CreateNopFuncJoinSafe)
{
	Pthread<void *, PthreadBehavior::Nothing> thread;
	void *val;

	thread.create(__routine_CreateNopFuncJoinSafe, nullptr);
	val = thread.join();

	asm volatile ("nop" : : "r" (val));
}


static void *__routine_CreateNopFuncJoinAuto(void *)
{
	return NULL;
}

Model(CreateNopFuncJoinAuto)
{
	Overhead(pthread_t tid, pthread_t tid = {});
	int ret;

	ret = pthread_create(&tid, NULL, __routine_CreateNopFuncJoinAuto,NULL);

	if (ret != 0) [[unlikely]]
		return;

	pthread_join(tid, NULL);
}
Test(CreateNopFuncJoinAuto)
{
	Pthread<void, PthreadBehavior::Join> thread;

	thread.create(__routine_CreateNopFuncJoinAuto, nullptr);
}


static void *__routine_CreatePassFuncJoinSafe(void *arg)
{
	return arg;
}

Model(CreatePassFuncJoinSafe)
{
	Overhead(pthread_t tid, pthread_t tid = {});
	void *in = (void *) 0x1234;
	void *out;
	int ret;

	ret = pthread_create(&tid, NULL, __routine_CreatePassFuncJoinSafe, in);

	if (ret != 0) [[unlikely]]
		return;

	pthread_join(tid, &out);

	asm volatile ("nop" : : "r" (out));
}
Test(CreatePassFuncJoinSafe)
{
	Pthread<void *, PthreadBehavior::Nothing> thread;
	void *in = (void *) 0x1234;
	void *out;

	thread.create(__routine_CreatePassFuncJoinSafe, in);
	out = thread.join();

	asm volatile ("nop" : : "r" (out));
}


static void __routine_CreateSubJoinSafe()
{
	asm volatile ("nop");
}

Model(CreateSubJoinSafe)
{
	Overhead(pthread_t tid, pthread_t tid = {});
	int ret;

	ret = pthread_create(&tid, NULL,
			     (void *(*)(void *)) __routine_CreateSubJoinSafe,
			     NULL);

	if (ret != 0) [[unlikely]]
		return;

	pthread_join(tid, NULL);
}
Test(CreateSubJoinSafe)
{
	Pthread<void, PthreadBehavior::Join> thread;

	thread.create(__routine_CreateSubJoinSafe);
}


Model(CreateSubJoinUnsafe)
{
	Overhead(pthread_t tid, pthread_t tid = {});

	pthread_create(&tid, NULL,
		       (void *(*)(void *)) __routine_CreateSubJoinSafe,
		       NULL);

	pthread_join(tid, NULL);
}
Test(CreateSubJoinUnsafe)
{
	Pthread<void, PthreadBehavior::Join> thread;

	thread.create(__routine_CreateSubJoinSafe, [](auto){});
}


static void __routine_CreateSubDetachAuto()
{
	asm volatile ("nop");
}

Model(CreateSubDetachAuto)
{
	Overhead(pthread_t tid, pthread_t tid = {});
	int ret;

	ret = pthread_create(&tid, NULL,
			     (void *(*)(void *)) __routine_CreateSubDetachAuto,
			     NULL);

	if (ret != 0) [[unlikely]]
		return;

	pthread_detach(tid);
}
Test(CreateSubDetachAuto)
{
	Pthread<void, PthreadBehavior::Detach> thread;

	thread.create(__routine_CreateSubDetachAuto);
}
