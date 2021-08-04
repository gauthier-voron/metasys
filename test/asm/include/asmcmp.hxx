#ifndef _INCLUDE_ASMTEST_HXX_
#define _INCLUDE_ASMTEST_HXX_


static inline void __before_prologue()
{
	asm volatile ("cli");
}

static inline void __after_epilogue()
{
	asm volatile ("hlt");
}


#define Model(name)							\
	static void __asmmodel_ ## name ();				\
									\
	void asmmodel_ ## name ()					\
	{								\
		__before_prologue();					\
		__asmmodel_ ## name ();					\
		__after_epilogue();					\
	}								\
									\
	static inline void __asmmodel_ ## name ()			\


#define Test(name)							\
	static void __asmtest_ ## name ();				\
									\
	void asmtest_ ## name ()					\
	{								\
		__before_prologue();					\
		__asmtest_ ## name ();					\
		__after_epilogue();					\
	}								\
									\
	static inline void __asmtest_ ## name ()			\


#ifdef ASMCMP_STRICT
#  define Overhead(should, does)  should
#else
#  define Overhead(should, does)  does
#endif


int main()
{
	return 1;
}


#endif
