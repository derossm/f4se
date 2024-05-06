#pragma once

class TESObjectREFR;

#include "f4se_common/Relocation.h"
#include "f4se/NiTypes.h"

using _CreateHandleByREFR = bool(*)(UInt32& handleOut, const TESObjectREFR* ref);
extern RelocAddr<_CreateHandleByREFR> CreateHandleByREFR;

using _LookupREFRByHandle = bool(*)(const UInt32& handleIn, NiPointer<TESObjectREFR>& ref);
extern RelocAddr<_LookupREFRByHandle> LookupREFRByHandle;

extern RelocPtr<UInt32> g_invalidRefHandle;

template<UInt32 INDEX_BITS = 20, UInt32 FLAG_BITS = 6>
class BSUntypedPointerHandle
{
public:
	using value_type = UInt32;

	BSUntypedPointerHandle() noexcept = default;

	BSUntypedPointerHandle(const BSUntypedPointerHandle&) noexcept = default;
	BSUntypedPointerHandle& operator=(const BSUntypedPointerHandle&) noexcept = default;

	BSUntypedPointerHandle(BSUntypedPointerHandle&&) noexcept = default;
	BSUntypedPointerHandle& operator=(BSUntypedPointerHandle&&) noexcept = default;

	BSUntypedPointerHandle(value_type handle) noexcept : _handle{handle} {}
	BSUntypedPointerHandle& operator=(value_type rhs) noexcept { _handle = rhs; return *this; }

	~BSUntypedPointerHandle() noexcept = default;

	[[nodiscard]] explicit operator bool() const noexcept { return has_value(); }
	[[nodiscard]] operator UInt32&() noexcept { return _handle; }
	[[nodiscard]] value_type value() const noexcept { return _handle; }

	void reset() noexcept { _handle = get_null_handle(); }

	[[nodiscard]] bool has_value() const noexcept { return _handle != get_null_handle(); }

	[[nodiscard]] bool operator==(const BSUntypedPointerHandle& rhs) const noexcept { return value() == rhs.value(); }
	[[nodiscard]] bool operator!=(const BSUntypedPointerHandle& rhs) const noexcept { return !(*this == rhs); }

	[[nodiscard]] bool operator==(value_type rhs) const noexcept { return value() == rhs; }
	[[nodiscard]] bool operator!=(value_type rhs) const noexcept { return !(*this == rhs); }

private:
	static UInt32 get_null_handle() noexcept
	{
		return *g_invalidRefHandle;
	}

	// members
	UInt32 _handle{get_null_handle()}; // 0
};
STATIC_ASSERT(sizeof(BSUntypedPointerHandle<>) == 0x4);

template <class T, class Handle = BSUntypedPointerHandle<>>
class BSPointerHandle : public Handle
{
public:
	using native_handle_type = typename Handle::value_type;

	BSPointerHandle() noexcept = default;
	BSPointerHandle(const BSPointerHandle&) noexcept = default;
	BSPointerHandle(BSPointerHandle&&) noexcept = default;

	BSPointerHandle& operator=(const BSPointerHandle& rhs)
	{
		Handle::operator=(rhs);
		return *this;
	}

	BSPointerHandle& operator=(BSPointerHandle&& rhs)
	{
		Handle::operator=(std::move(rhs));
		return *this;
	}

	~BSPointerHandle() noexcept = default;

	template<class Y>
	explicit BSPointerHandle(const Y* rhs)
		: Handle()
	{
		if (rhs && rhs->BSHandleRefObject::QRefCount() > 0)
		{
			create(rhs);
		}
	}

	template<class Y>
	BSPointerHandle(const BSPointerHandle<Y, Handle>& rhs)
		: Handle(rhs)
	{
	}

	template <class Y>
	BSPointerHandle(BSPointerHandle<Y, Handle>&& rhs)
		: Handle(std::move(rhs))
	{
	}

	template<class Y>
	BSPointerHandle& operator=(const Y* rhs)
	{
		if (rhs && rhs->handleRefObject.QRefCount() > 0)
		{
			create(rhs);
		}
		else
		{
			reset();
		}

		return *this;
	}

	template<class Y>
	BSPointerHandle& operator=(const BSPointerHandle<Y, Handle>& rhs)
	{
		Handle::operator=(static_cast<const Handle&>(rhs));
		return *this;
	}

	void reset() { Handle::reset(); }

	NiPointer<T> get() const
	{
		NiPointer<T> ptr;
		lookup(ptr);
		return ptr;
	}

	native_handle_type native_handle() { return Handle::value(); }

	operator bool() const { return Handle::has_value(); }

	bool operator==(const BSPointerHandle& rhs) const { return static_cast<const Handle&>(*this) == static_cast<const Handle&>(rhs); }
	bool operator!=(const BSPointerHandle& rhs) const { return !(*this == rhs); }

private:
	void create(const T* ptr)
	{
		CreateHandleByREFR(*this, ptr);
	}

	bool lookup(NiPointer<T>& refPtr) const
	{
		return LookupREFRByHandle(*this, refPtr);
	}
};

template <class T>
class BSPointerHandleManager
{
public:
};
STATIC_ASSERT(sizeof(BSPointerHandleManager<void*>) == 0x1);

class HandleManager : public BSPointerHandleManager<BSUntypedPointerHandle<>>
{
public:
};
STATIC_ASSERT(sizeof(HandleManager) == 0x1);

template <class T, class Manager = HandleManager>
class BSPointerHandleManagerInterface
{
public:
	using value_type = T;
};

template <class T>
class BSPointerHandleSmartPointer : public NiPointer<typename T::value_type>
{
public:
};
