#ifndef SSVU_FASTFUNC
#define SSVU_FASTFUNC

#include <cstring>
#include <type_traits>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <utility>

namespace ssvu
{
	namespace Internal
	{
		class AnyClass;
		using AnyPtrThis = AnyClass*;
		using AnyPtrFunc = void(AnyClass::*)();
		template<typename TReturn = void, typename... TArgs> using AnyPtrFuncT = TReturn(AnyClass::*)(TArgs...);
		template<typename TReturn = void, typename... TArgs> using AnyPtrStaticFuncT = TReturn(*)(TArgs...);

		constexpr std::size_t SingleMemFuncPtrSize{sizeof(void(AnyClass::*)())};

		template<class TOut, class TIn> union HorribleUnion { TOut out; TIn in; };
		template<class TOut, class TIn> inline TOut horrible_cast(TIn mIn) noexcept { HorribleUnion<TOut, TIn> u; static_assert(sizeof(TIn) == sizeof(u) && sizeof(TIn) == sizeof(TOut), "Cannot use horrible_cast<>"); u.in = mIn; return u.out; }
		template<class TOut, class TIn> inline TOut unsafe_horrible_cast(TIn mIn) noexcept { HorribleUnion<TOut, TIn> u; u.in = mIn; return u.out; }

		template<std::size_t TN> struct SimplifyMemFunc
		{
			template<class TThis, class TFunc> inline static AnyPtrThis convert(const TThis*, TFunc, AnyPtrFunc&) noexcept
			{
				static_assert(TN - 100, "Unsupported member function pointer on this compiler");
				return 0;
			}
		};
		template<> struct SimplifyMemFunc<SingleMemFuncPtrSize>
		{
			template<class TThis, class TFunc> inline static AnyPtrThis convert(const TThis* mThis, TFunc mFunc, AnyPtrFunc& mFuncOut) noexcept
			{
				mFuncOut = reinterpret_cast<AnyPtrFunc>(mFunc);
				return reinterpret_cast<AnyPtrThis>(const_cast<TThis*>(mThis));
			}
		};

		template<typename TReturn, typename... TArgs> struct Closure
		{
			private:
				using PtrFuncT = AnyPtrFuncT<TReturn, TArgs...>;
				using PtrStaticFuncT = AnyPtrStaticFuncT<TReturn, TArgs...>;
				AnyPtrThis ptrThis{nullptr};
				AnyPtrFunc ptrFunction{nullptr};

			public:
				template<class TThis, class TFunc> inline void bind(TThis* mThis, TFunc mFunc) noexcept { ptrThis = SimplifyMemFunc<sizeof(mFunc)>::convert(mThis, mFunc, ptrFunction); }
				template<class TThis, class TInvoker> inline void bind(TThis* mThis, TInvoker mInvoker, PtrStaticFuncT mFunc) noexcept
				{
					if(mFunc == nullptr) ptrFunction = nullptr; else bind(mThis, mInvoker);
					ptrThis = horrible_cast<AnyPtrThis>(mFunc);
				}

				inline bool operator==(std::nullptr_t) const noexcept		{ return ptrThis == nullptr && ptrFunction == nullptr; }
				inline bool operator==(const Closure& mRhs) const noexcept	{ return ptrThis == mRhs.ptrThis && ptrFunction == mRhs.ptrFunction; }
				inline bool operator==(PtrStaticFuncT mPtr) const noexcept	{ return mPtr == nullptr ? *this == nullptr : mPtr == reinterpret_cast<PtrStaticFuncT>(getStaticFunc()); }
				inline bool operator!=(std::nullptr_t) const noexcept		{ return !operator==(nullptr); }
				inline bool operator!=(const Closure& mRhs) const noexcept	{ return !operator==(mRhs); }
				inline bool operator!=(PtrStaticFuncT mPtr) const noexcept	{ return !operator==(mPtr); }
				inline bool operator<(const Closure& mRhs) const			{ return ptrThis != mRhs.ptrThis ? ptrThis < mRhs.ptrThis : std::memcmp(&ptrFunction, &mRhs.ptrFunction, sizeof(ptrFunction)) < 0; }
				inline bool operator>(const Closure& mRhs) const			{ return !operator<(mRhs); }

				inline std::size_t getHash() const noexcept					{ return reinterpret_cast<std::size_t>(ptrThis) ^ Internal::unsafe_horrible_cast<std::size_t>(ptrFunction); }
				inline AnyPtrThis getPtrThis() const noexcept				{ return ptrThis; }
				inline PtrFuncT getPtrFunction() const noexcept				{ return reinterpret_cast<PtrFuncT>(ptrFunction); }
				inline PtrStaticFuncT getStaticFunc() const noexcept		{ return horrible_cast<PtrStaticFuncT>(this); }
		};

		template<typename TReturn, typename... TArgs> class FastFuncImpl
		{
			private:
				using PtrStaticFuncT = AnyPtrStaticFuncT<TReturn, TArgs...>;
				Closure<TReturn, TArgs...> closure;
				inline TReturn invokeStaticFunc(TArgs... mArgs) const { return (*(closure.getStaticFunc()))(std::forward<TArgs>(mArgs)...); }

			protected:
				template<class TThis, class TFunc> inline void bind(TThis* mThis, TFunc mFunc) noexcept { closure.bind(mThis, mFunc); }
				template<class TFunc> inline void bind(TFunc mFunc) noexcept { closure.bind(this, &FastFuncImpl::invokeStaticFunc, mFunc); }

			public:
				inline FastFuncImpl() noexcept = default;
				inline FastFuncImpl(std::nullptr_t) noexcept { }
				inline FastFuncImpl(PtrStaticFuncT mFunc) noexcept { bind(mFunc); }
				template<typename X, typename Y> inline FastFuncImpl(X* mThis, Y mFunc) noexcept { bind(mThis, mFunc); }


				inline FastFuncImpl& operator=(PtrStaticFuncT mFunc) noexcept	{ bind(mFunc); }
				inline TReturn operator()(TArgs... mArgs) const					{ return (closure.getPtrThis()->*(closure.getPtrFunction()))(std::forward<TArgs>(mArgs)...); }

				inline bool operator==(std::nullptr_t) const noexcept				{ return closure == nullptr; }
				inline bool operator==(const FastFuncImpl& mImpl) const noexcept	{ return closure == mImpl.closure; }
				inline bool operator==(PtrStaticFuncT mFuncPtr) const noexcept		{ return closure == mFuncPtr; }
				inline bool operator!=(std::nullptr_t) const noexcept				{ return !operator==(nullptr); }
				inline bool operator!=(const FastFuncImpl& mImpl) const noexcept	{ return !operator==(mImpl); }
				inline bool operator!=(PtrStaticFuncT mFuncPtr) const noexcept		{ return !operator==(mFuncPtr); }
				inline bool operator<(const FastFuncImpl& mImpl) const				{ return closure < mImpl.closure; }
				inline bool operator>(const FastFuncImpl& mImpl) const				{ return !operator<(mImpl); }
		};
	}

	template<typename T> struct MemFuncToFunc;
	template<typename TReturn, typename TThis, typename... TArgs> struct MemFuncToFunc<TReturn(TThis::*)(TArgs...) const> { using Type = TReturn(*)(TArgs...); };

	#define ENABLE_IF_CONV_TO_FUN_PTR(x)		typename std::enable_if<std::is_constructible<typename MemFuncToFunc<decltype(&std::decay<x>::type::operator())>::Type, x>::value>::type* = nullptr
	#define ENABLE_IF_NOT_CONV_TO_FUN_PTR(x)	typename std::enable_if<!std::is_constructible<typename MemFuncToFunc<decltype(&std::decay<x>::type::operator())>::Type, x>::value>::type* = nullptr
	#define ENABLE_IF_SAME_TYPE(x, y)			typename = typename std::enable_if<!std::is_same<x, typename std::decay<y>::type>{}>::type

	template<typename T> class FastFunc;
	template<typename TReturn, typename... TArgs> class FastFunc<TReturn(TArgs...)> : public Internal::FastFuncImpl<TReturn, TArgs...>
	{
		private:
			using BaseType = Internal::FastFuncImpl<TReturn, TArgs...>;
			std::shared_ptr<void> storage;
			template<typename T> inline static void funcDeleter(void* mPtr) { static_cast<T*>(mPtr)->~T(); operator delete(mPtr); }

		public:
			using BaseType::BaseType;

			inline FastFunc() noexcept = default;

			template<typename TFunc, ENABLE_IF_SAME_TYPE(FastFunc, TFunc)> inline FastFunc(TFunc&& mFunc, ENABLE_IF_CONV_TO_FUN_PTR(TFunc))
			{
				using FuncType = typename std::decay<TFunc>::type;
				this->bind(&mFunc, &FuncType::operator());
			}
			template<typename TFunc, ENABLE_IF_SAME_TYPE(FastFunc, TFunc)> inline FastFunc(TFunc&& mFunc, ENABLE_IF_NOT_CONV_TO_FUN_PTR(TFunc))
				: storage(operator new(sizeof(TFunc)), funcDeleter<typename std::decay<TFunc>::type>)
			{
				using FuncType = typename std::decay<TFunc>::type;
				new (storage.get()) FuncType(std::forward<TFunc>(mFunc));
				this->bind(storage.get(), &FuncType::operator());
			}
	};

	#undef ENABLE_IF_CONV_TO_FUN_PTR
	#undef ENABLE_IF_NOT_CONV_TO_FUN_PTR
	#undef ENABLE_IF_SAME_TYPE
}

#endif