/*
	Input and output stream classes for comedilib's C++ binding.
	The << and >> operators are NOT useful, as the overloaded bitshift
	operators are used for formatting.  Instead use the unformatted read() and
	write() member functions.

	Copyright (C) 2007  Frank Mori Hess <fmhess@users.sourceforge.net>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _COMEDILIB_IOSTREAM_HPP
#define _COMEDILIB_IOSTREAM_HPP

#include <comedilib.hpp>
#include <cstdio>
#include <ext/stdio_filebuf.h>
#include <iostream>
#include <unistd.h>

namespace comedi
{
	namespace detail
	{
		class filebuf
		{
		public:
			filebuf(const comedi::device &dev, std::ios::openmode mode, size_t buffer_size)
			{
				int fd = dup(dev.fileno());
				if(fd < 0)
				{
					throw std::invalid_argument(__PRETTY_FUNCTION__);
				}
				m_filebuf.reset(new __gnu_cxx::stdio_filebuf<char>(fd, mode, buffer_size));
			}
			boost::scoped_ptr<__gnu_cxx::stdio_filebuf<char> > m_filebuf;
		};
	}

	class istream: private detail::filebuf, public std::istream
	{
	public:
		istream(const comedi::device &dev, size_t buffer_size = static_cast<size_t>(BUFSIZ)):
			detail::filebuf(dev, std::ios::in | std::ios::binary, buffer_size),
			std::istream(this->m_filebuf.get())
		{}
	};

	class ostream: private detail::filebuf, public std::ostream
	{
	public:
		ostream(const comedi::device &dev, size_t buffer_size = static_cast<size_t>(BUFSIZ)):
			detail::filebuf(dev, std::ios::out | std::ios::binary, buffer_size),
			std::ostream(this->m_filebuf.get())
		{}
	};
};

#endif	// _COMEDILIB_IOSTREAM_HPP
