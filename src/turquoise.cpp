
#include "turquoise.hpp"


namespace turquoise
{

Executor::Executor(ExecutorPool& executorPool, std::uint16_t index):
    executorPool(executorPool),
    index(index)
{
}

void Executor::operator()()
{

}

void Executor::proceed()
{
    executorPool.proceed();
}


// === ExecutorPool ====================

ExecutorPool::ExecutorPool(): ExecutorPool(std::thread::hardware_concurrency())
{
}

ExecutorPool::ExecutorPool(std::size_t numExecutors)
{
    for (std::size_t i = 0; i < numExecutors; ++i)
    {
        executors.emplace_back(*this, static_cast<std::uint16_t>(i));
    }
}

void ExecutorPool::run()
{
    for (Executor& executor: executors)
    {
        std::thread thread{[&executor](){
            executor();
        }};

        thread.detach();
    }
}

void ExecutorPool::proceed()
{

}


void Executor::notify()
{
    {
        std::lock_guard<std::mutex> lockGuard(mutex);
    }
    conditionVariable.notify_one();
}


bool ExecutorPool::notifyExecutorAndUnlock(std::size_t index, PromiseResultSupplier* promiseResultSupplier)
{
    if (!activeExecutor)
    {
        unlock();
        executors[index].notify();
        return true;
    }
    else
    {
        unlock();
        executors[index].
    }

    return false;
}


}
