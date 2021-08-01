// Copyright 2021 Gauthier Voron
//
// This file is part of Metasys.
//
// Metasys is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Metasys is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// Metasys. If not, see <https://www.gnu.org/licenses/>.
//


#ifndef _INCLUDE_METASYS_SCHED_PTHREADBEHAVIOR_HXX_
#define _INCLUDE_METASYS_SCHED_PTHREADBEHAVIOR_HXX_


#include <cstdint>


namespace metasys {


namespace details {


struct PthreadBehaviorImpl
{
	static constexpr uint16_t CALLER_MASK  = 0x007;
	static constexpr uint16_t NOTHING      = 0x001;
	static constexpr uint16_t TERMINATE    = 0x002;
	static constexpr uint16_t DETACH       = 0x003;
	static constexpr uint16_t JOIN         = 0x004;

	static constexpr uint16_t SIGNAL_MASK  = 0xff8;
	static constexpr uint16_t CANCEL       = 0x008;
	static constexpr uint16_t KILL_SHIFT   = 4;
	static constexpr uint16_t KILL_MASK    = 0xff0;


	uint16_t  value;


	constexpr PthreadBehaviorImpl(uint16_t _value) noexcept
		: value(_value)
	{
	}
};


constexpr PthreadBehaviorImpl operator|(PthreadBehaviorImpl a,
					PthreadBehaviorImpl b)
{
	if (((a.value & PthreadBehaviorImpl::CALLER_MASK) != 0) &&
	    ((b.value & PthreadBehaviorImpl::CALLER_MASK) != 0)) {
		throw "two caller behaviors specified";
	}

	if (((a.value & PthreadBehaviorImpl::SIGNAL_MASK) != 0) &&
	    ((b.value & PthreadBehaviorImpl::SIGNAL_MASK) != 0)) {
		throw "two signal behaviors specified";
	}

	return PthreadBehaviorImpl(a.value | b.value);
}


}


struct PthreadBehavior
{
	using Impl = details::PthreadBehaviorImpl;


	Impl  impl;


	constexpr PthreadBehavior(Impl _impl) noexcept
		: impl(_impl)
	{
		if ((impl.value & Impl::CALLER_MASK) == 0)
			impl = impl | Terminate;
	}


	constexpr bool operator&(Impl t) const
	{
		if ((t.value & Impl::CALLER_MASK) != 0) {
			if ((t.value & ~Impl::CALLER_MASK) != 0)
				throw "test for exclusive properties";
			return ((t.value & Impl::CALLER_MASK) ==
				(impl.value & Impl::CALLER_MASK));
		} else if ((t.value & Impl::SIGNAL_MASK) != 0) {
			if ((t.value & ~Impl::SIGNAL_MASK) != 0)
				throw "test for exclusive properties";
			else if (t.value == Impl::CANCEL)
				return ((impl.value & Impl::SIGNAL_MASK) ==
					Impl::CANCEL);
			else if ((t.value & Impl::KILL_MASK) != 0)
				return ((impl.value & Impl::KILL_MASK) ==
					(t.value & Impl::KILL_MASK));
			else
				throw "test for unknown signal property";
		} else {
			throw "test for unknown property";
		}
	}

	constexpr int signum() const noexcept
	{
		return ((impl.value & Impl::KILL_MASK) >> Impl::KILL_SHIFT);
	}


	static constexpr Impl Nothing   { Impl::NOTHING   };
	static constexpr Impl Terminate { Impl::TERMINATE };
	static constexpr Impl Detach    { Impl::DETACH    };
	static constexpr Impl Join      { Impl::JOIN      };

	static constexpr Impl Cancel    { Impl::CANCEL    };

	static constexpr Impl Kill(int sig = SIGTERM) noexcept
	{
		return Impl((sig << Impl::KILL_SHIFT) & Impl::KILL_MASK);
	}
};


}


#endif
