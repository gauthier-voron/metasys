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


#ifndef _INCLUDE_METASYS_SCHED_PTHREADCANCELINHIBITOR_HXX_
#define _INCLUDE_METASYS_SCHED_PTHREADCANCELINHIBITOR_HXX_


#include <metasys/sched/ThisPthread.hxx>


namespace metasys {


class PthreadCancelInhibitor
{
	bool  _old;


 public:
	PthreadCancelInhibitor() noexcept
		: _old(ThisPthread::setcancel(false))
	{
	}

	PthreadCancelInhibitor(const PthreadCancelInhibitor &) = delete;
	PthreadCancelInhibitor(PthreadCancelInhibitor &&) = delete;

	~PthreadCancelInhibitor() noexcept (false)
	{
		ThisPthread::setcancel(_old);
	}

	PthreadCancelInhibitor &operator=(const PthreadCancelInhibitor &) =
		delete;
	PthreadCancelInhibitor *operator=(PthreadCancelInhibitor &&) = delete;
};


}


#endif
