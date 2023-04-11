#pragma once

#include "common/AlignedStorage.h"
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace chisel
{
    using ErrorCodeBaseType = int32_t;

    enum class BasicErrorCode : ErrorCodeBaseType
    {
        Invalid  = -1,
        Failed   = -2,
        NotFound = -3,

        Success =  0,
    };

    struct VoidResult
    {
    };

    template <typename T, typename ErrorCode = BasicErrorCode, ErrorCode SuccessCode = BasicErrorCode::Success, ErrorCode InvalidCode = BasicErrorCode::Invalid, ErrorCode DefaultFail = BasicErrorCode::Failed>
    class Result
    {
    public:
        ~Result() { Destroy(); }

        T* Get()
        {
            //Assert(IsSuccess());
            if (!IsSuccess())
                return nullptr;

            return &RefUnsafe();
        }

        const T* Get() const
        {
            Assert(IsSuccess());
            if (!IsSuccess())
                return nullptr;

            return &RefUnsafe();
        }

        bool IsSuccess() const { return static_cast<int32_t>(m_error) >= 0; }

        operator bool() const { return IsSuccess(); }

              T* operator ->()       { return Get(); }
        const T* operator ->() const { return Get(); }

              T& operator *()        { return *Get(); }
        const T& operator *()  const { return *Get(); }

        static Result Error(ErrorCode code = DefaultFail)
        {
            Result res;
            res.MakeError(code);
            return res;
        }

        template <typename... Args>
        static Result PrintError(const char* message, Args&&... args)
        {
            //log::err(message, std::forward<Args>(args)...);
            return Error();
        }

        template <typename... Args>
        static Result PrintError(ErrorCode code, const char* message, Args&&... args)
        {
            //log::err(message, std::forward<Args>(args)...);
            return Error(code);
        }

        template <typename... Args>
        static Result Success(Args&&... args)
        {
            Result res;
            res.Create(std::forward<Args>(args)...);
            res.MakeSuccess();
            return res;
        }

        ErrorCode Code() const { return m_error; }

        template <typename OtherResult>
        static Result ForwardError(OtherResult& x)
        {
            return Error(x.Code());
        }

        template <typename OtherResult, typename... Args>
        static Result PrintForwardError(OtherResult& x, const char* message, Args&&... args)
        {
            //log::err(message, std::forward<Args>(args)...);
            return Error(x.Code());
        }

    protected:
        Result() = default;

        template <typename... Args>
        Result& Create(Args&&... args)
        {
            //Assert(!m_created);
            m_created = true;
            new(reinterpret_cast<void*>(&m_data)) T{ std::forward<Args>(args)... };
            return *this;
        }

        void Destroy()
        {
            if (m_created)
            {
                RefUnsafe().~T();
                m_created = false;
            }
        }

        Result& MakeError(ErrorCode code) { m_error = code;        return *this; }
        Result& MakeSuccess()             { m_error = SuccessCode; return *this; }

              T& RefUnsafe()       { return *reinterpret_cast<      T*>(&m_data); }
        const T& RefUnsafe() const { return *reinterpret_cast<const T*>(&m_data); }

        friend T;
        AlignedStorage<sizeof(T), alignof(T)> m_data;
        ErrorCode                             m_error = InvalidCode;
        bool                                  m_created = false;
    };

    // It's like a reference, but allows
    // it to be nullptr once, at constructor time
    // and never again.
    template <typename T>
    class WeakRef
    {
    public:
        WeakRef() {}
        WeakRef(std::nullptr_t) = delete;
        WeakRef(T& ref)
            : m_ptr{ &ref } { }

        WeakRef(const WeakRef& other)
            : m_ptr { other.m_ptr } {}

        WeakRef& operator = (T& object)
        {
            m_ptr = &object;
            return *this;
        }

        WeakRef& operator = (const WeakRef& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }
        
        WeakRef& operator = (std::nullptr_t) = delete;

        T* operator -> () const { return m_ptr; }

        T**       operator & ()       { return &m_ptr; }
        T* const* operator & () const { return &m_ptr; }

        bool operator == (const WeakRef& other) const { return m_ptr == other.m_ptr; }
        bool operator != (const WeakRef& other) const { return m_ptr != other.m_ptr; }

        bool operator == (const T* other) const { return m_ptr == other; }
        bool operator != (const T* other) const { return m_ptr != other; }

        bool operator == (std::nullptr_t) const { return m_ptr == nullptr; }
        bool operator != (std::nullptr_t) const { return m_ptr != nullptr; }

    private:
        T* m_ptr = nullptr;
    };
}
