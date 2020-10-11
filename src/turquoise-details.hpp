// Copyright Andrey Lifanov 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifndef __TURQUOISE_DETAILS_HPP__
#define __TURQUOISE_DETAILS_HPP__

#include <list>


namespace turquoise::details
{

template <class T, std::size_t binCapacity = 128>
class Queue
{
    struct Bin
    {
        unsigned int size;
        unsigned int readIndex;

        struct EmptyByte {};
        union U {
            U(): empty() {}
            EmptyByte empty;
            T data[binCapacity];
        } data;

        Bin(): size(0), readIndex(0)
        {
        }

        ~Bin()
        {
            for (std::size_t i = 0; i < size; ++i)
            {
                data.data[i].~T();
            }
        }

        [[nodiscard]] bool isEmpty() const noexcept
        {
            return size == 0;
        }

        [[nodiscard]] bool isFull() const noexcept
        {
            return size == binCapacity;
        }

        void push(T&& value)
        {
            new(data.data + size) T(std::move(value));
            ++size;
        }

        void push(const T& value)
        {
            new(data.data + size) T(value);
            ++size;
        }

        T pop()
        {
            T result = std::move(data.data[readIndex]);
            ++readIndex;
            return result;
        }

        [[nodiscard]] bool canPop() const noexcept
        {
            return readIndex < size;
        }
    };

public:
    Queue(): size_(0), readBin_(bins_.end()), writeBin_(bins_.end())
    {
    }

    [[nodiscard]] std::size_t getSize() const noexcept
    {
        return size_;
    }

    [[nodiscard]] bool isEmpty() const noexcept
    {
        return size_ == 0;
    }

    void prepareWriteBin()
    {
        if (writeBin_ == bins_.end() || writeBin_->isFull())
        {
            writeBin_ = bins_.emplace(bins_.end());
        }
    }

    void push(T&& element)
    {
        prepareWriteBin();
        writeBin_->push(std::move(element));
        ++size_;
    }

    void push(const T& element)
    {
        prepareWriteBin();
        writeBin_->push(element);
        --size_;
    }

    T pop()
    {
        if (readBin_ != bins_.end())
        {
            if (!readBin_->canPop())
            {
                bins_.erase(readBin_);
                readBin_ = bins_.begin();
            }
        }
        else
        {
            readBin_ = bins_.begin();
        }

        T result = readBin_->pop();
        return result;
    }

private:
    std::list<Bin> bins_;
    std::size_t size_;
    using Iterator = typename std::list<Bin>::iterator;
    Iterator readBin_;
    Iterator writeBin_;
};

} // ns

#endif // include guard
