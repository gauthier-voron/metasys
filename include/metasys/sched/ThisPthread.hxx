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


#ifndef _INCLUDE_METASYS_SCHED_THISPTHREAD_HXX_
#define _INCLUDE_METASYS_SCHED_THISPTHREAD_HXX_


#include <pthread.h>

#include <cassert>


namespace metasys {


struct ThisPthread
{
	static void testcancel()
	{
		::pthread_testcancel();
	}


	static bool setcancel(bool enabled) noexcept
	{
		int state, oldstate, ret;

		if (enabled)
			state = PTHREAD_CANCEL_ENABLE;
		else
			state = PTHREAD_CANCEL_DISABLE;

		ret = ::pthread_setcancelstate(state, &oldstate);

		assert(ret == 0);

		if (oldstate == PTHREAD_CANCEL_ENABLE)
			return true;
		else
			return false;
	}
};


}


#endif
