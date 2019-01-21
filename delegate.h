#pragma once
/*
 * Copyright 2019
 * Authored by: Ben Diamand
 *
 * English version - you can use this for whatever you want.  Attribution much
 * appreciated but not required.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as part of a compiled
 * binary, for any purpose, commercial or non-commercial, and by any means,
 * subject to the following conditions(s):
 *
 * ** This comment block must remain in this and derived works.
 */

#pragma once
#include <array>
#include <type_traits>
#include <utility>

/**
 *                                                  ^^^ Rationale ^^^
 *
 * There are many  examples of std::function replacements, but I was unable to find something that did exactly what
 * these classes do.  For example, this page has a comparison of a plethora of std::function replacements:
 * https://github.com/jamboree/CxxFunctionBenchmark.
 *
 * The design goals for this implementation are:
 *      Predictable memory and runtime -> fixed size and heapless
 *      Simple to use with no possible runtime errors, asserts, etc.
 *      Fast
 *      Small
 *
 * Fixed (size) delegates are a std::function alternative, with more speed / space performance but less functionality.
 * The differences compared to std:function are:
 *      (Pro) More performant in space and speed than std::function
 *      (Pro) Fixed size, never allocates
 *      (Pro) Never any uninitialized state
 *      (Pro) Can choose to make trivialy constructed delegates, or full featured ones with slightly more memory
 *      (Pro) Documented
 *      (Pro / Con) No explicit support for RTTI or exceptions (no knowledge of exceptions at all)
 *      (Pro / Con) Similar syntax to std::function
 *      (Con) Unable to store arbitrary sized captures - captures must fit the compile-time delegate size
 *      (Con) Unable to return all types (in particular reference types, or any others not default constructable)
 *
 * Note: See the accompanying unit tests for some good examples of use.
 */

namespace Delegate
{
    /*
     * Allow the delegate size to be specified as a compile-time constant.
     */
    #ifndef DELEGATE_ARGS_SIZE
     #define DELEGATE_ARGS_SIZE sizeof(int) + sizeof(int *)
     #define DELEGATE_ARGS_SIZE_UNDEF
    #endif
    #ifndef DELEGATE_ARGS_ALIGN
     #define DELEGATE_ARGS_ALIGN 8
     #define DELEGATE_ARGS_ALIGN_UNDEF
    #endif

    /**
     * Templated class representing the aligned storage of a delegate.  The intent is for all delegates to have the
     * same size, and this information is purposefully not part of the delegate signature.
     *
     * @tparam size Number of bytes of storage per delegate.
     * @tparam alignement How to align the data.
     */
    template <size_t size = DELEGATE_ARGS_SIZE, size_t alignment = DELEGATE_ARGS_ALIGN>
    struct TemplateFunctorArgs
    {
    private:
        /**
         * The actual storage.
         */
        alignas(alignment) std::array<char, size> args;
    };

    #ifdef DELEGATE_ARGS_SIZE_UNDEF
     #undef DELEGATE_ARGS_SIZE
    #endif
    #ifdef DELEGATE_ARGS_ALIGN_UNDEF
     #undef DELEGATE_ARGS_ALIGN
    #endif

    /**
     * A simplifying typedef to make the code more readable.
     */
    using FunctorArgs = TemplateFunctorArgs<>;

    /**
     * Determine whether there is enough space to hold the delegate.
     * 
     * @return Returns true if there is enough space, else false.
     */
    template<typename T>
    constexpr bool can_emplace()
    {
        return (sizeof(T) <= sizeof(FunctorArgs)) &&
               (std::alignment_of<FunctorArgs>::value % std::alignment_of<T>::value) == 0;
    }

    /**
     * Reimbues a type-erased piece of memory with its original functor type.
     *
     * @tparam T The functor type.
     * @param args The memory to reimbue.
     *
     * @return Returns a reference to the (now properly typed) memory.
     */
    template<typename T>
    static T &get_typed_functor(const FunctorArgs &args)
    {
        return (T &)(args);
    }

    /**
     * Store a functor and any associated captured data into a piece of type-erased memory.
     * 
     * @tparam T The functor type.
     * @param args The memory to store to.
     * @to_store The memory to store from.
     */
    template<typename T>
    static void store_functor(FunctorArgs &args, const T &to_store)
    {
        new (&get_typed_functor<T>(args)) T(to_store);
    }

    /**
     * Move a functor and any associated captured data into a piece of type-erased memory.
     * 
     * @tparam T The functor type.
     * @param args The memory to store into.
     * @param to_move The type to move.
     */
    template<typename T>
    static void move_functor(FunctorArgs &args, T &&to_move)
    {
        new (&get_typed_functor<T>(args)) T(std::move(to_move));
    }

    /**
     * Call the type-erased functor (with the correct type).  This is a pure forwarding function, only passing along
     * arguments to the actual functor, i.e. a trampoline to call the real functor.
     * 
     * @tparam T The functor type.
     * @tparam Result The return type.
     * @tparam Arguments The functor argument types.
     * @param args The functor memory (i.e. function pointer or captures).
     * @param arguments The functor arguments.
     *
     * @return The functor return type.
     */
    template<typename T, typename Result, typename... Arguments>
    static Result typed_call(const FunctorArgs &args, Arguments&&... arguments)
    {
        return get_typed_functor<T>(args)(std::forward<Arguments>(arguments)...);
    }

    /**
     * Class used to specialize the delegate.  This one is used to select the trivial delegate.
     */
    class TrivialType
    {
    };

    /**
     * Class used to specialize the delegate.  This one is used to select the non-trivial delegate.  However,
     * this is not actually directly instantiatable.
     */
    class NonTrivialType
    {
    };

    /**
     * Class used to specialize the delegate.  This one is used to select non-movable types, the most flexible kind.
     */
    class NonMovableType
    {
    };

    /**
     * Class used to specialize the delegate.  This one is used to select non-copyable types, the most restrictive kind.
     */
    class NonCopyableType
    {
    };

    /**
     * Unspecialized (and unused) delegate class.
     */
    template<typename Type, typename Result, typename... Arguments>
    class Func
    {
        Func() = delete;
    };

    /**
     * Another (smaller) name for the type-erased call function.
     */
    template<typename Result, typename... Arguments>
    using func_call = Result (*)(const FunctorArgs &args, Arguments&&... arguments);

    /**
     * A trivial delegate - trivially constructable and copyable.  This delegate is smaller than a non-trivial one, and
     * is allowed to treat copies as memcpy and elide destructor calls.  All that's needed if all captures are POD-like.
     *
     * @tparam Result The delegate return type - must be default constructable.
     * @tparam Arguments The delegate function arguments.
     */
    template<typename Result, typename... Arguments>
    class Func<TrivialType, Result, Arguments...>
    {
    public:
        /**
         * Default (delegating) constructor.
         */
        Func() :
            Func([](Arguments&&...){return Result();})
        {
        }

        /**
         * Copy Constructor.
         *
         * @param other The delegate to copy.
         */
        Func(const Func &other):
            args(other.args),
            call(other.call)
        {
        }

        /**
         * Constructor.
         *
         * Note: Args is left uninitialized until store_functor is called in the constructor body.
         *
         * @tparam T The functor type.
         * @param functor The functor / function to call.
         */
        template<typename T>
        Func(const T &functor) :
             call(&typed_call<T, Result, Arguments...>)
        {
            static_assert(can_emplace<T>(), "Delegate doesn't fit.");
            store_functor(args, functor);
        }

        /**
         * Assignment operator.  Pass by value because delegates are small and construction has no side efects.
         *
         * @param other The delegate to copy from.
         *
         * @return Returns a reference to the class.
         */
        Func &operator=(Func other)
        {
            args = other.args;
            call = other.call;

            return *this;
        }

        /**
         * Forwarding function call operator.
         *
         * @param arguments The arguments to pass through to the delegate.
         *
         * @return Returns the Result type.
         */
        Result operator()(Arguments... arguments) const
        {
            return call(args, std::forward<Arguments>(arguments)...);
        }

        /**
         * @return the type-erased call function.
         */
        func_call<Result, Arguments...> get_call() const
        {
            return call;
        }

        /**
         * @return Returns the delegate argument memory block.
         */
        FunctorArgs get_args() const
        {
            return args;
        }

    private:
        /**
         * The delegate arguments (function pointers and / or captures go here).
         */
        FunctorArgs args;

        /**
         * Trampoline function which reimbues the type-erased delgate with its original type and calls the functor.
         * This is statically constructed by the compiler or copied from another such value and cannot be null.
         */
        func_call<Result, Arguments...> call;
    };

    /**
     * Manual virtual table implementation.  A virtual table is useful because there are multiple functions a full
     * delegate has beyond the call function (copy and deletion), and storing pointers for each type erased function
     * would make the delegate larger for no real gain (delegates are expected to be called far more than copied or
     * moved).  As for why actual virtual functions aren't used, a manual table allows the virtual table to be swapped,
     * allowing the nice property that a full delegate can copy / construct from a trivial one and work identically.
     */
    struct Vtable
    {
        /**
         * Emits a full function static table pointer, unique to the template parameter.
         *
         * @tparam T The functor type associated with the virtual table.
         */
        template<typename T>
        inline static const Vtable &get_vtable()
        {
            static const Vtable vtable = 
            {
                typed_copy<T>,
                typed_move<T>,
                typed_destroy<T>
            };

            return vtable;
        }

        /**
         * Emits a trivial function static table pointer, unique to the template parameter.
         *
         * @tparam T The functor type associated with the virtual table.
         */
        template<typename = void>
        inline static const Vtable &empty_vtable()
        {
            static const Vtable vtable = 
            {
                empty_copy,
                empty_move,
                empty_destroy
            };

            return vtable;
        }

        /**
         * Reference to the copy function.
         */
        void (& copy)(FunctorArgs &lhs, const FunctorArgs &rhs);

        /**
         * Reference to the move function.
         */
        void (& move)(FunctorArgs &lhs, FunctorArgs &&rhs);

        /**
         * Reference to the destroy function.
         */
        void (& destroy)(FunctorArgs &args);

        /**
         * Actual code to perform a copy.
         *
         * @tparam T The functor type to copy.
         * @param lhs The reference to receive the copied data.
         * @param lhs The reference to provide the copied data.
         */
        template<typename T,
                 typename std::enable_if<std::is_copy_constructible<T>::value, T>::type* = nullptr>
        static void typed_copy(FunctorArgs &lhs, const FunctorArgs &rhs)
        {
            store_functor<T>(lhs, get_typed_functor<T>(rhs));
        }

        /**
         * Dummy copy.
         *
         * This function exists because there are functors which cannot be copy constructed.  For example, unique_ptr
         * doesn't allow itself to be copied.  If a delegate has a unique_ptr value capture, the delegate can no longer
         * be copied to another delegate; that would violate the promise unique_ptr makes that there will exist only one
         * actual pointer value.
         *
         * @tparam T The functor type to copy.
         * @param lhs The reference to receive the copied data.
         * @param lhs The reference to provide the copied data.
         */
        template<typename T,
                 typename std::enable_if<!std::is_copy_constructible<T>::value, T>::type* = nullptr>
        static void typed_copy(FunctorArgs &, const FunctorArgs &)
        {
        }

        /**
         * Actual code to perform a move.
         *
         * @tparam T The functor type to move.
         * @param lhs The reference to receive the moved data.
         * @param lhs The reference to provide the moved data.
         */
        template<typename T>
        static void typed_move(FunctorArgs &lhs, FunctorArgs &&rhs)
        {
            move_functor<T>(lhs, std::move(get_typed_functor<T>(rhs)));
        }

        /**
         * Actual code to perform a destroy.
         *
         * @tparam T The functor type to copy.
         * @param args The memory of the functor type to destroy.
         */
        template<typename T>
        static void typed_destroy(FunctorArgs &args)
        {
            get_typed_functor<T>(args).~T();
        }

        /**
         * Empty copy function, for trivial delegates.
         */
        static void empty_copy(FunctorArgs &, const FunctorArgs &)
        {
        }

        /**
         * Empty move function, for trivial delegates.
         */
        static void empty_move(FunctorArgs &, FunctorArgs &&)
        {
        }

        /**
         * Empty destroy function, for trivial delegates.
         */
        static void empty_destroy(FunctorArgs &)
        {
        }
    };

    /**
     * A non-trivial delegate - in which all captured types are property constructed and destroyed.
     *
     * Note that this is a perfectly functional class, and passes all unit tests.  It's also an inherently unsafe class,
     * because it cannot prevent a non-copyable lambda (e.g. one capturing a unique_ptr) from being copied via a copy of
     * the delegate.  For that reason, it's not itself constructable, and the user must choose between either the
     * non-movable (most flexible) and non-copyable (most restrictive) variants.
     *
     * @tparam Result The delegate return type - must be default constructable.
     * @tparam Arguments The delegate function arguments.
     */
    template<typename Result, typename... Arguments>
    class Func<NonTrivialType, Result, Arguments...>
    {
    protected:
        /**
         * Copy Constructor for the trivial variant.
         *
         * @param other The delegate to copy.
         */
        Func(const Func<TrivialType, Result, Arguments...> &other):
             call(other.get_call()),
             vtable(&Vtable::empty_vtable<>())
        {
            args = other.get_args();
        }

        /**
         * Default constructor.  Note that args is left uninitialized since it's unused here.
         */
        Func() :
            call([](const FunctorArgs &, Arguments&&...){return Result();}),
            vtable(&Vtable::empty_vtable<>())
        {
        }

        /**
         * Functor copy constructor.
         *
         * @tparam T The functor type.
         * @param functor The functor to copy from.
         */
        template<typename T>
        Func(const T &functor) :
            call(&typed_call<T, Result, Arguments...>),
            vtable(&Vtable::get_vtable<T>())
        {
            static_assert(can_emplace<T>(), "Delegate doesn't fit.");
            store_functor(args, functor);
        }

        /**
         * Functor move constructor.
         *
         * @tparam T The functor type.
         * @param functor The functor to move.
         */
        template<typename T>
        Func(T &&functor) :
            call(&typed_call<T, Result, Arguments...>),
            vtable(&Vtable::get_vtable<T>())
        {
            move_functor(args, std::move(functor));
        }

        /**
         * Copy Constructor.
         *
         * @param other The delegate to copy.
         */
        Func(const Func &other) :
            call(other.call),
            vtable(other.vtable)
        {
            vtable->copy(args, other.args);
        }

        /**
         * Move constructor.
         *
         * @param other The delegate to move from.
         */
        Func(Func &&other) :
            call(other.call),
            vtable(other.vtable)
        {
            other.vtable->move(args, std::move(other.args));

            /*
             * Leave other in some well defined state.  Use a named temporary so the move assignment below isn't called.
             */
            const Func tmp;
            other = tmp;
        }

        /**
         * Move assignment operator.
         * 
         * @param other The delegate to move from.
         * 
         * @return Returns a reference to this.
         */
        Func &operator=(Func &&other)
        {
            /*
             * Self move is non-sensical, so no check here for that.
             */
            vtable->destroy(args);
            other.vtable->move(args, std::move(other.args));
            this->call = other.call;
            this->vtable = other.vtable;

            /*
             * Leave other in some well defined state.  Use a named temporary to keep this function from being called.
             */
            const Func tmp;
            other = tmp;

            return *this;
        }

        /**
         * Copy assignment operator.
         *
         * @param other The delegate to copy from.
         *
         * @return Returns a reference to this.
         */
        Func &operator=(const Func &other)
        {
            if (this == &other)
            {
                return *this;
            }

            vtable->destroy(args);
            other.vtable->copy(args, other.args);
            this->call = other.call;
            this->vtable = other.vtable;

            return *this;
        }

    public:
        /**
         * Forwarding function call operator.
         *
         * @param arguments The arguments to pass through to the delegate.
         *
         * @return Returns the Result type.
         */
        Result operator()(Arguments... arguments) const
        {
            return call(args, std::forward<Arguments>(arguments)...);
        }

        /**
         * Destructor.
         */
        ~Func()
        {
            vtable->destroy(args);
        }

    private:
        /**
         * The delegate arguments (function pointers and / or captures go here).
         */
        FunctorArgs args;

        /**
         * Trampoline function which reimbues the type-erased delgate with its original type and calls the functor.
         * This is statically constructed by the compiler or copied from another value and cannot be null.
         */
        func_call<Result, Arguments...> call;

        /**
         * Pointer to the manual virtual table.  This is statically constructed by the compiler or copied from another
         * value and cannot be null.
         */
        const Vtable *vtable;
    };

    /**
     * Overload of Func which restricts users to only non-movable capture types.
     *
     * All functions are pure pass through to the base class, and only exist to make them public.
     *
     * @tparam Result The return type.
     * @tparam Arguments The functor argument types.
     */
    template<typename Result, typename... Arguments>
    class Func<NonMovableType, Result, Arguments...> : public Func<NonTrivialType, Result, Arguments...>
    {
    public:
        Func() = default;

        Func(const Func &other) = default;
        
        Func &operator=(const Func &other) = default;

        template<typename T>
        Func(const T &functor) : Func<NonTrivialType, Result, Arguments...>(functor)
        {
        }
    };

    /**
     * Overload of Func which restricts users to only non-copyable capture types.
     *
     * All functions are pure pass through to the base class, and only exist to make them public.
     *
     * @tparam Result The return type.
     * @tparam Arguments The functor argument types.
     */
    template<typename Result, typename... Arguments>
    class Func<NonCopyableType, Result, Arguments...> : public Func<NonTrivialType, Result, Arguments...>
    {
    public:
        Func() = default;

        Func(Func &&other) = default;

        Func &operator=(Func &&other) = default;

        template<typename T>
        Func(T &&functor) : Func<NonTrivialType, Result, Arguments...>(std::move(functor))
        {
        }
    };

    template<typename Result, typename... Arguments>
    using FuncTrivial = Func<TrivialType, Result, Arguments...>;

    template<typename Result, typename... Arguments>
    using FuncNonCopy = Func<NonCopyableType, Result, Arguments...>;

    template<typename Result, typename... Arguments>
    using FuncNonMove = Func<NonMovableType, Result, Arguments...>;
}
