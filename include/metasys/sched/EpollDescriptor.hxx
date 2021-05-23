#ifndef _INCLUDE_METASYS_SCHED_EPOLLDESCRIPTOR_HXX_
#define _INCLUDE_METASYS_SCHED_EPOLLDESCRIPTOR_HXX_


#include <sys/epoll.h>

#include <cassert>
#include <cerrno>
#include <concepts>

#include <metasys/sys/ClosingDescriptor.hxx>
#include <metasys/sys/SystemException.hxx>


namespace metasys {


namespace details {


template<typename T, typename E>
concept IsTrivialArray = requires (T a)
{
	{ a.size() } -> std::same_as<size_t>;
	{ a.data() } -> std::convertible_to<E *>;
};


}


template<typename T>
requires std::is_pointer_v<T>
      || std::same_as<T, int>
      || std::same_as<T, uint32_t>
      || std::same_as<T, uint64_t>
class EpollEvent : public epoll_event
{
 public:
	constexpr EpollEvent() noexcept = default;

	constexpr EpollEvent(uint32_t events, T data) noexcept
		: epoll_event({})
	{
		epoll_event::events = events;

		if constexpr (std::is_pointer_v<T>) {
			epoll_event::data.ptr = (void *) data;
		} if constexpr (std::same_as<T, int>) {
			epoll_event::data.fd = data;
		} else if constexpr (std::same_as<T, uint32_t>) {
			epoll_event::data.u32 = data;
		} else if constexpr (std::same_as<T, uint64_t>) {
			epoll_event::data.u64 = data;
		}
	}

	constexpr EpollEvent(uint32_t events, const FileDescriptor &fd) noexcept
		: EpollEvent(fd.value())
	{
	}

	constexpr EpollEvent(const EpollEvent &other) noexcept = default;

	EpollEvent(EpollEvent &&other) noexcept = default;


	constexpr uint32_t events() const noexcept
	{
		return epoll_event::events;
	}

	constexpr T data() const noexcept
	{
		if constexpr (std::is_pointer_v<T>) {
			return (T) epoll_event::data.ptr;
		} else if constexpr (std::same_as<T, int>) {
			return epoll_event::data.fd;
		} else if constexpr (std::same_as<T, uint32_t>) {
			return epoll_event::data.u32;
		} else if constexpr (std::same_as<T, uint64_t>) {
			return epoll_event::data.u64;
		}
	}


	epoll_event &c_event() noexcept
	{
		return *this;
	}
};


class EpollDescriptor : public ClosingDescriptor
{
 public:
	EpollDescriptor() noexcept
		: ClosingDescriptor()
	{
	}

	EpollDescriptor(int fd) noexcept
		: ClosingDescriptor(fd)
	{
	}

	EpollDescriptor(FileDescriptor &&other) noexcept
		: ClosingDescriptor(std::move(other))
	{
	}


	template<typename ErrHandler>
	auto create(ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		assert(valid() == false);

		reset(::epoll_create(1));

		return handler(value());
	}

	void create()
	{
		create([](int ret) {
			if (ret < 0) [[unlikely]]
				throwcreate();
		});
	}

	template<typename ErrHandler>
	static EpollDescriptor createinit(ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		int fd = ::epoll_create(1);

		handler(fd);

		return EpollDescriptor(fd);
	}

	static EpollDescriptor createinit()
	{
		return createinit([](int ret) {
			if (ret < 0) [[unlikely]]
				throwcreate();
		});
	}

	static void throwcreate()
	{
		assert(errno != EINVAL);

		SystemException::throwErrno();
	}


	template<typename ErrHandler>
	auto ctl(int op, int fd, struct epoll_event *event, ErrHandler &&err)
		noexcept (noexcept (err(-1)))
	{
		assert(valid());

		return err(::epoll_ctl(value(), op, fd, event));
	}

	void ctl(int op, int fd, struct epoll_event *event)
	{
		ctl(op, fd, event, [](int ret) {
			if (ret < 0) [[unlikely]]
				throwctl();
		});
	}

	template<typename ... Args>
	auto ctl(int op, const FileDescriptor &fd, Args && ... args)
		noexcept (noexcept (ctl(op, fd.value(),
					std::forward<Args>(args) ...)))
	{
		return ctl(op, fd.value(), std::forward<Args>(args) ...);
	}

	template<typename Fd, typename E, typename ... Args>
	auto ctl(int op, Fd &&fd, EpollEvent<E> *event, Args && ... args)
	{
		return ctl(op, std::forward<Fd>(fd), &event->c_event(),
			   std::forward<Args>(args) ...);
	}

	static void throwctl()
	{
		assert(errno != EBADF);
		assert(errno != EEXIST);
		assert(errno != EINVAL);
		assert(errno != EPOLLEXCLUSIVE);
		assert(errno != ELOOP);
		assert(errno != ENOENT);
		assert(errno != EPERM);

		SystemException::throwErrno();
	}


 private:
	// Note: The manpages indicate that the `event` field in `epoll_ctl()`
	//       is of type `struct epoll_event *` and not of type
	//       `const struct epoll_event *`.
	//       However in practice, this field is never modified.
	//       Kernel maintainers indicate that they leave the field mutable
	//       to allow adding more operations to `epoll_ctl()` which could
	//       modify the field.
	//       It is then safe to use `const` version for existing operations
	//       namely `EPOLL_CTL_ADD`, `EPOLL_CTL_MOD` and `EPOLL_CTL_DEL`.
	//       Here are the private wrappers accepting const references.

	template<typename Fd, typename ... Args>
	auto _ctl_maybe_constev(int op, Fd &&fd,
				const struct epoll_event &event,
				Args && ... args)
		noexcept (noexcept (ctl(op,
			std::forward<Fd>(fd),
			const_cast<struct epoll_event *> (&event),
			std::forward<Args>(args) ...)))
	{
		return ctl(op, std::forward<Fd>(fd),
			   const_cast<struct epoll_event *> (&event),
			   std::forward<Args>(args) ...);
	}

	template<typename Fd, typename E, typename ... Args>
	auto _ctl_maybe_constev(int op, Fd &&fd, const EpollEvent<E> &event,
				Args && ... args)
		noexcept (noexcept (ctl(op, std::forward<Fd>(fd),
			&const_cast<EpollEvent<E> *> (&event)->c_event(),
			std::forward<Args>(args) ...)))
	{
		return ctl(op, std::forward<Fd>(fd),
			   &const_cast<EpollEvent<E> *> (&event)->c_event(),
			   std::forward<Args>(args) ...);
	}


 public:
	template<typename ... Args>
	auto add(Args && ... args)
		noexcept (noexcept (_ctl_maybe_constev(EPOLL_CTL_ADD,
			std::forward<Args>(args) ...)))
	{
		return _ctl_maybe_constev(EPOLL_CTL_ADD,
					  std::forward<Args>(args) ...);
	}

	template<typename ... Args>
	auto mod(Args && ... args)
		noexcept (noexcept (_ctl_maybe_constev(EPOLL_CTL_MOD,
			std::forward<Args>(args) ...)))
	{
		return _ctl_maybe_constev(EPOLL_CTL_MOD,
					  std::forward<Args>(args) ...);
	}

	template<typename ... Args>
	auto del(Args && ... args)
		noexcept (noexcept (_ctl_maybe_constev(EPOLL_CTL_DEL,
			std::forward<Args>(args) ...)))
	{
		return _ctl_maybe_constev(EPOLL_CTL_DEL,
					  std::forward<Args>(args) ...);
	}


	template<typename ErrHandler>
	auto wait(struct epoll_event *events, uint32_t maxevents, int timeout,
		  ErrHandler &&handler)
		noexcept (noexcept (handler(-1)))
	{
		assert(valid());

		return handler(::epoll_wait(value(), events, maxevents,
					    timeout));
	}

	template<typename ErrHandler>
	auto wait(struct epoll_event *events, uint32_t maxevents,
		  ErrHandler &&handler) noexcept (noexcept (handler(-1)))
	{
		return wait(events, maxevents, -1,
			    std::forward<ErrHandler>(handler));
	}

	size_t wait(struct epoll_event *events, uint32_t maxevents,
		    int timeout = -1) noexcept
	{
		int ret;

		assert(valid());

	retry:
		ret = ::epoll_wait(value(), events, maxevents, timeout);

		if (ret < 0) [[unlikely]] {
			if (errno == EINTR)
				goto retry;
			assert(errno != EBADF);
			assert(errno != EFAULT);
			assert(errno != EINVAL);
		}

		return ((size_t) ret);
	}

	static void throwwait()
	{
		assert(errno != EBADF);
		assert(errno != EFAULT);
		assert(errno != EINVAL);

		SystemException::throwErrno();
	}

	template<typename D, typename ... Args>
	auto wait(EpollEvent<D> *events, uint32_t maxevents, Args && ... args)
		noexcept (noexcept (wait(
			reinterpret_cast<struct epoll_event *> (events),
			maxevents, std::forward<Args>(args) ...)))
	{
		static_assert (sizeof (EpollEvent<D>)
			       == sizeof (struct epoll_event));

		return wait(reinterpret_cast<struct epoll_event *> (events),
			    maxevents, std::forward<Args>(args) ...);
	}

	template<typename Container, typename ... Args>
	requires details::IsTrivialArray<Container, struct epoll_event>
	auto wait(Container *dest, Args && ... args)
		noexcept (noexcept (wait(reinterpret_cast<struct epoll_event *>
			(dest->data()), dest->size(),
			std::forward<Args>(args) ...)))
	{
		return wait(reinterpret_cast<struct epoll_event *>
			    (dest->data()),
			    dest->size(), std::forward<Args>(args) ...);
	}
};


}


#endif
