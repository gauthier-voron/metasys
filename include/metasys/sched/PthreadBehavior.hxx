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


namespace detail {


struct PthreadBehaviorImpl
{
	uint8_t  value;


	static constexpr uint8_t BASE_MASK = 3;
	static constexpr uint8_t NOTHING   = 0;
	static constexpr uint8_t TERMINATE = 1;
	static constexpr uint8_t DETACH    = 2;
	static constexpr uint8_t JOIN      = 3;

	static constexpr uint8_t OPTION_MASK = 4;
	static constexpr uint8_t CANCEL      = 4;


	constexpr PthreadBehaviorImpl(uint8_t _value) noexcept
		: value(_value)
	{
	}
};


struct PthreadBehaviorBase
{
	uint8_t  value;


	constexpr PthreadBehaviorBase(uint8_t _value) noexcept
		: value(_value)
	{
	}
};


struct PthreadBehaviorCancel
{
};


constexpr PthreadBehaviorImpl operator|(PthreadBehaviorBase b,
				        PthreadBehaviorCancel) noexcept
{
	return { static_cast<uint8_t> (b.value|PthreadBehaviorImpl::CANCEL) };
}

constexpr PthreadBehaviorImpl operator|(PthreadBehaviorCancel,
					PthreadBehaviorBase b) noexcept
{
	return { static_cast<uint8_t> (b.value|PthreadBehaviorImpl::CANCEL) };
}


}


struct PthreadBehavior
{
	using Impl       = detail::PthreadBehaviorImpl;
	using Base       = detail::PthreadBehaviorBase;
	using CancelType = detail::PthreadBehaviorCancel;


	Impl  impl;


	constexpr PthreadBehavior(Base _base) noexcept
		: impl(_base.value)
	{
	}

	constexpr PthreadBehavior(Impl _impl) noexcept
		: impl(_impl)
	{
	}


	constexpr bool operator==(Base base) const noexcept
	{
		return ((impl.value & Impl::BASE_MASK) == base.value);
	}

	constexpr bool operator&(CancelType) const noexcept
	{
		return ((impl.value & Impl::CANCEL) != 0);
	}


	static constexpr Base Nothing   { Impl::NOTHING   };
	static constexpr Base Terminate { Impl::TERMINATE };
	static constexpr Base Detach    { Impl::DETACH    };
	static constexpr Base Join      { Impl::JOIN      };

	static constexpr CancelType Cancel {};
};


}


#endif
