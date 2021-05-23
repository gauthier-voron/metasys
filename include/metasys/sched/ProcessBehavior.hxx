#ifndef _INCLUDE_METASYS_SCHED_PROCESSBEHAVIOR_HXX_
#define _INCLUDE_METASYS_SCHED_PROCESSBEHAVIOR_HXX_


#include <csignal>


namespace metasys {


struct ProcessBehavior
{
	bool CaptureStdin   = false;
	bool CaptureStdout  = false;
	bool CaptureStderr  = false;

	int SignalOnDestroy = 0;

	bool WaitOnDestroy  = false;


	static constexpr ProcessBehavior DEFAULT() noexcept
	{
		return ProcessBehavior();
	}

	static constexpr ProcessBehavior SUBPROCESS() noexcept
	{
		return ProcessBehavior({
			.SignalOnDestroy = SIGTERM,
			.WaitOnDestroy = true
		});
	}

	static constexpr ProcessBehavior COMMAND() noexcept
	{
		return ProcessBehavior({
			.CaptureStdout = true,
			.SignalOnDestroy = SIGTERM,
			.WaitOnDestroy = true
		});
	}

	static constexpr ProcessBehavior PIPED() noexcept
	{
		return ProcessBehavior({
			.CaptureStdin = true,
			.SignalOnDestroy = SIGTERM,
			.WaitOnDestroy = true
		});
	}

	static constexpr ProcessBehavior SERVICE() noexcept
	{
		return ProcessBehavior({
			.CaptureStdin = true,
			.CaptureStdout = true,
			.CaptureStderr = true,
			.SignalOnDestroy = SIGTERM,
			.WaitOnDestroy = true
		});
	}
};


}


#endif
