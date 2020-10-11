// Copyright Andrey Lifanov 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifndef __TURQUOISE__HPP__
#define __TURQUOISE__HPP__

#include "turquoise-details.hpp"

#include <memory>
#include <exception>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <cstdint>
#include <limits>


namespace turquoise
{

// === Forward declarations

class Task;
class Executor;
class ExecutorPool;

template <class T>
class Future;

template <class T>
class Promise;

///

class Error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
    ~Error() override = default;
};

class PromiseError : public Error
{
public:
    explicit PromiseError(const std::string& error):
        Error(std::string{"Promise error: "} + error)
    {
    }

    ~PromiseError() override = default;
};

class FutureError : public Error
{
public:
    explicit FutureError(const std::string& error):
            Error(std::string{"Future error: "} + error)
    {
    }

    ~FutureError() override = default;
};

class PromiseResultSupplier
{
public:
    enum State : std::uint16_t
    {
        EmptyResult,
        ValueResult,
        ErrorResult
    };

    PromiseResultSupplier() noexcept:
        state_(EmptyResult),
        servedFuturesReferenceCounter_(0),
        servedPromisesReferenceCounter_(1), // created by Promise
        executorIndex_(std::numeric_limits<decltype(executorIndex_)>::max())
    {
    }

    [[nodiscard]] bool canDelete() const noexcept
    {
        return servedFuturesReferenceCounter_ == 0 && servedPromisesReferenceCounter_ == 0;
    }

    void increaseFuturesRC() noexcept { ++servedFuturesReferenceCounter_; }
    void decreaseFuturesRC() noexcept { --servedFuturesReferenceCounter_; }
    void increasePromisesRC() noexcept { ++servedPromisesReferenceCounter_; }
    void decreasePromisesRC() noexcept { --servedPromisesReferenceCounter_; }

    [[nodiscard]] bool isReady() const noexcept
    {
        return state_ == ValueResult || state_ == ErrorResult;
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        return state_ == EmptyResult && servedFuturesReferenceCounter_ > 0;
    }

    [[nodiscard]] bool isAwaitingExecutor() const noexcept
    {
        return executorIndex_ < std::numeric_limits<decltype(executorIndex_)>::max();
    }

    virtual ~PromiseResultSupplier() = default;

protected:
    State state_;
    std::uint16_t servedFuturesReferenceCounter_;
    std::uint16_t servedPromisesReferenceCounter_;
    std::uint16_t executorIndex_;
};

template <class T>
class PromiseResultSupplierTemplate final : public PromiseResultSupplier
{
    friend class Future<T>;
    friend class Promise<T>;

    struct EmptyByte {};
    union {
        EmptyByte emptyByte;
        std::exception_ptr exceptionPtr;
        T value;
    } result_;

    PromiseResultSupplierTemplate() noexcept:
        PromiseResultSupplier(),
        result_(EmptyByte{})
    {
    }

    void setResult(T result, ExecutorPool& executorPool);

    void setException(const std::exception_ptr& e)
    {
        result_.exceptionPtr = e;
        state_ = ErrorResult;
    }

public:
    ~PromiseResultSupplier() final
    {
        if (state_ == ValueResult)
        {
            result_.value.~T();
        }
        else if (state_ == ErrorResult)
        {
            result_.exceptionPtr.~exception_ptr();
        }
    }
};


template <class T>
class Future
{
    using Self = Future;
    friend class Promise<T>;
public:
    Future(const Self& another) = delete;
    Future(Self&& another) noexcept = default;
    Self& operator=(const Self& another) = delete;
    Self& operator=(Self&& another) noexcept = default;

    void wait(Executor& executor)
    {

    }

    T get()
    {
        if (promiseResultSupplier->exceptionPtr)
        {
            std::rethrow_exception(promiseResultSupplier->exceptionPtr);
        }

        return std::move(promiseResultSupplier->value.value());
    }

private:
    explicit Future(PromiseResultSupplierTemplate<T>* resultSupplier) noexcept:
            promiseResultSupplier(resultSupplier)
    {
        resultSupplier->increaseReferenceCounter();
    }

    PromiseResultSupplierTemplate<T>* promiseResultSupplier;
};

template <class T>
class Promise
{
    using Self = Promise;
public:
    explicit Promise(ExecutorPool& executorPool):
        executorPool_(executorPool),
        promiseResultSupplier(new PromiseResultSupplierTemplate<T>)
    {
    }

    Promise(const Self& another) = delete;
    Promise(Self&& another) noexcept = default;
    Self& operator=(const Self& another) = delete;
    Self& operator=(Self&& another) noexcept = default;

    Future<T> getFuture() const
    {
        return {promiseResultSupplier};
    }

    void setResult(T&& result)
    {
        promiseResultSupplier->setResult(std::move(result));
    }

    void setResult(const T& result)
    {
        promiseResultSupplier->setResult(result);
    }

    void setExceptionPointer(std::exception_ptr e)
    {
        promiseResultSupplier->setException(e);
    }

    template <class E>
    void setException(E exception)
    {
        setExceptionPointer(std::make_exception_ptr(std::move(exception)));
    }

private:
    ExecutorPool& executorPool_;
    PromiseResultSupplierTemplate<T>* promiseResultSupplier;
};

class Task
{
public:
    virtual void operator()(Executor&) = 0;
    virtual ~Task() = default;
};



class Executor
{
    friend class ExecutorPool;
public:
    template<class Future>
    auto wait(Future& future)
    {
        proceed();
        future.wait();
        waitForActivation();
        return future.get();
    }

private:
    Executor(ExecutorPool& executorPool, std::uint16_t index);
    void operator()();
    void proceed();
    void waitForActivation();
    void notify();

private:
    ExecutorPool& executorPool;
    std::uint16_t index;
    std::condition_variable conditionVariable;
    std::mutex mutex;
    std::vector<PromiseResultSupplier*> awaitingResults;
};

class ExecutorPool
{
    friend class Executor;
public:
    ExecutorPool();
    ExecutorPool(std::size_t numExecutors);
    void run();

    /*Executor& operator[](std::size_t index)
    {
        return executors[index];
    }*/
    template <class Func>
    auto postTask(Func f)
    {
        
    }

    void lock()
    {
        mutex.lock();
    }

    void unlock()
    {
        mutex.unlock();
    }

    bool notifyExecutorAndUnlock(std::size_t index, PromiseResultSupplier*);

private:
    void proceed();

private:
    std::vector<Executor> executors;
    Executor* activeExecutor;
    details::Queue<Task> tasks;
    std::mutex mutex;
};

// === Details ==============================================================================

template <class T>
void PromiseResultSupplierTemplate<T>::setResult(T result, ExecutorPool& executorPool)
{
    executorPool.lock();
    result_.value = std::move(result);
    state_ = ValueResult;

    if (isAwaitingExecutor())
    {
        executorPool.notifyExecutorAndUnlock(executorIndex_, this));
    }
    else
    {
        executorPool.unlock();
    }
}

} // ns

#endif // include guard
