#ifndef _INCLUDE_METASYS_IO_PIPE_HXX_
#define _INCLUDE_METASYS_IO_PIPE_HXX_


#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>

#include <metasys/io/ReadableDescriptor.hxx>
#include <metasys/io/WritableDescriptor.hxx>
#include <metasys/sys/ClosingDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


class Pipe
{
	int  _fds[2];


	struct noinit_t
	{
		constexpr noinit_t() noexcept = default;
	};

	static constexpr noinit_t NoInit {};

	Pipe(noinit_t) noexcept
	{
	}


 public:
	Pipe() noexcept
		: _fds{-1, -1}
	{
	}

	Pipe(const Pipe &) = delete;

	Pipe(Pipe &&other) noexcept
	{
		_fds[0] = other._fds[0];
		_fds[1] = other._fds[1];
		other._fds[0] = -1;
		other._fds[1] = -1;
	}

	[[gnu::always_inline]]
	~Pipe()
	{
		if (rend().valid())
			rend().close([](auto){});
		if (wend().valid())
			wend().close([](auto){});
	}


	Pipe &operator=(const Pipe &) = delete;

	Pipe &operator=(Pipe &&other)
	{
		if (rend().valid()) {
			assert(rend().value() != other.wend().value());

			if (rend().value() != other.rend().value()) [[likely]]
				rend().close();
		}

		if (wend().valid()) {
			assert(wend().value() != other.rend().value());

			if (wend().value() != other.wend().value()) [[likely]]
				wend().close();
		}

		_fds[0] = other._fds[0];
		_fds[1] = other._fds[1];

		other._fds[0] = -1;
		other._fds[1] = -1;

		return *this;
	}


	class ReaderProxy : public ReadableDescriptor
	{
		friend class Pipe;


		explicit ReaderProxy(int fd) noexcept
			: ReadableDescriptor(fd)
		{
		}


	 public:
		ReaderProxy &operator=(const ReaderProxy &) = delete;
		ReaderProxy &operator=(ReaderProxy &&) = delete;
	};

	ReaderProxy rend() noexcept
	{
		return ReaderProxy(_fds[0]);
	}


	class WriterProxy : public WritableDescriptor
	{
		friend class Pipe;


		explicit WriterProxy(int fd) noexcept
			: WritableDescriptor(fd)
		{
		}


	 public:
		WriterProxy &operator=(const WriterProxy &) = delete;
		WriterProxy &operator=(WriterProxy &&) = delete;
	};

	WriterProxy wend() noexcept
	{
		return WriterProxy(_fds[1]);
	}


	class Reader : public ReadableInterface<ClosingDescriptor>
	{
		friend class Pipe;


		explicit Reader(int fd) noexcept
			: ReadableInterface<ClosingDescriptor>(fd)
		{
		}


	 public:
		Reader() noexcept = default;
		Reader(const Reader &) = delete;
		Reader(Reader &&other) noexcept = default;

		Reader &operator=(const Reader &) = delete;
		Reader &operator=(Reader &&other) = default;
	};

	Reader rmove() noexcept
	{
		int tmp = _fds[0];

		_fds[0] = -1;

		return Reader(tmp);
	}


	class Writer : public WritableInterface<ClosingDescriptor>
	{
		friend class Pipe;


		explicit Writer(int fd) noexcept
			: WritableInterface<ClosingDescriptor>(fd)
		{
		}


	 public:
		Writer() noexcept = default;
		Writer(const Writer &) = delete;
		Writer(Writer &&other) noexcept = default;

		Writer &operator=(const Writer &) = delete;
		Writer &operator=(Writer &&other) = default;
	};

	Writer wmove() noexcept
	{
		int tmp = _fds[1];

		_fds[1] = -1;

		return Writer(tmp);
	}


	template<typename ErrHandler>
	auto open(ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		assert(rend().valid() == false);
		assert(wend().valid() == false);

		return handler(::pipe(_fds));
	}

	template<typename ErrHandler>
	static Pipe openinit(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		Pipe ret = Pipe(NoInit);

		handler(::pipe(ret._fds));

		return ret;
	}

	void open()
	{
		open([](int ret) {
			if (ret != 0) [[unlikely]]
				throwopen();
		});
	}

	static Pipe openinit()
	{
		Pipe ret = Pipe(NoInit);

		if (::pipe(ret._fds) != 0) [[unlikely]]
			throwopen();

		return ret;
	}


	template<typename ErrHandler>
	auto open(int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(rend().valid() == false);
		assert(wend().valid() == false);

		return handler(::pipe2(_fds, flags));
	}

	template<typename ErrHandler>
	static Pipe openinit(int flags, ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		Pipe ret = Pipe(NoInit);

		handler(::pipe2(ret._fds, flags));

		return ret;
	}

	void open(int flags)
	{
		open(flags, [](int ret) {
			if (ret != 0) [[unlikely]]
				throwopen();
		});
	}

	static Pipe openinit(int flags)
	{
		Pipe ret = Pipe(NoInit);

		if (::pipe2(ret._fds, flags) != 0) [[unlikely]]
			throwopen();

		return ret;
	}

	static void throwopen()
	{
		assert(errno != EFAULT);

		SystemException::throwErrno();
	}
};


}


#endif
