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


#ifndef _INCLUDE_METASYS_SCHED_THREAD_HXX_
#define _INCLUDE_METASYS_SCHED_THREAD_HXX_


#include <utility>

#include <metasys/sched/Pthread.hxx>
#include <metasys/sched/PthreadCancelInhibitor.hxx>
#include <metasys/sched/ThisPthread.hxx>


namespace metasys {


using Thread = Pthread<void, PthreadBehavior::Terminate>;

class Jthread
	: public Pthread<void, PthreadBehavior::Join | PthreadBehavior::Cancel>
{
 public:
	template<auto Value>
	struct Constexpr
	{
	};


	constexpr Jthread() noexcept = default;

	template<typename ... Args>
	Jthread(Args && ... args)
	{
		create(std::forward<Args>(args) ...);
	}

	template<auto M, typename ... Args>
	Jthread(Constexpr<M>, Args && ... args)
	{
		create<M>(std::forward<Args>(args) ...);
	}
};

using ThisThread = ThisPthread;

using ThreadCancelInhibitor = PthreadCancelInhibitor;


}


#endif
