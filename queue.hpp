#pragma once
#include <assert.h>
#include <iterator> 

namespace nstd {

// a circular queue that stores data contiguously.
// stores a back and front handle. data is added to the back handle which is incremented.
// if the size of the queue reaches the capacity, the queue is reallocated to double the size and the contents moved
// an index that exceeds the capacity will be rolled back to the front (hence the circular name). That means 
// it's not truly contiguous as iteration will have to roll back to the start. 
// has iterators
// not exception safe and a use it at your own risk design. 
// I don't use exceptions. However, if you want to, replace all the cases of abort() and asserts() with throws.
// by default, a signed int type is used to handle the size, capacity and the handles. I prefer this but some people don't so you can just change it
// since it's a templated parameter
// no copy constructors by design, you will write better code that way.
template <class T, typename INT_TYPE = int>
struct queue {
    static_assert(std::is_fundamental<INT_TYPE>(), "INT_TYPE is not an integer");
private:
    T* buffer_ = nullptr;
    INT_TYPE front_ = 0;
    INT_TYPE back_ = 0; // back is not inclusive, it is one element after the last element
    INT_TYPE capacity_ = 0;
    INT_TYPE size_ = 0;

    struct iterator {
        using iterator_category = std::forward_iterator_tag;
    private:
        T* buffer_;
        INT_TYPE front_;
        INT_TYPE capacity_;
        INT_TYPE offset_;
    public:
        iterator(T* buffer, INT_TYPE front, INT_TYPE offset, INT_TYPE capacity) : buffer_(buffer), front_(front), offset_(offset), capacity_(capacity) {}

        T& operator*() const { return buffer_[(offset_ + front_) % capacity_]; }
        T* operator->() { return &buffer_[(offset_ + front_) % capacity_]; }

        // Prefix increment
        iterator& operator++() { ++offset_; return *this; }

        // Postfix increment
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const iterator& a, const iterator& b) { return a.offset_ == b.offset_; };
        friend bool operator!= (const iterator& a, const iterator& b) { return a.offset_ != b.offset_; };
    };

public:

    queue() {}

    // deliberate. you don't need to copy these. write helper functions if you need to do that
    queue(const queue<T>& queue) = delete;
    queue<T>& operator=(const queue<T>& queue) = delete;
    queue<T>& operator=(queue<T>&& type) = delete;

    ~queue() {
        if (buffer_ == nullptr) return;

        // call the destructors
        for (INT_TYPE i = 0; i < size_; ++i) {
            INT_TYPE index_rolling = (front_ + i) % capacity_;
            buffer_[index_rolling].~T();
        }

        free(buffer_);
    }

private:

    void should_reallocate() {

        if (capacity_ == size_) {

            if (capacity_ == 0) capacity_ = 2;
            else capacity_ *= 2;

            T* buffer_new = (T*)malloc(sizeof(T) * capacity_);
            if (buffer_new == nullptr) abort();

            // copy old buffer into new buffer 
            // where we copy into the new buffer from it's
            // start point 
            for (INT_TYPE i = 0; i < size_; i++) {
                INT_TYPE index_rolling = (front_ + i) % size_;
                buffer_new[i] = std::move(buffer_[index_rolling]); // ensures we copy correctly (i think since move semantics are annoying and stupid)
            }

            // free the old buffer 
            free(buffer_);
            buffer_ = buffer_new;

            front_ = 0;
            back_ = size_;
        }
    }
public:

    iterator begin() {
        return iterator(buffer_, front_, 0, capacity_);
    }

    iterator end() {
        return iterator(buffer_, front_, size_, capacity_);
    }

    void clear() {
        while (size_ > 0) {
            pop();
        }
        front_ = 0;
        back_ = 0;
    }

    void push_back(const T& data) {
        should_reallocate();

        buffer_[back_] = data;
        back_ = (back_ + 1) % capacity_;
        ++size_;
    }

    T& emplace_back() {
        should_reallocate();

        T* data = new (&buffer_[back_]) T();
        if (data == nullptr) abort();

        back_ = (back_ + 1) % capacity_;
        ++size_;
        return *data;
    }

    void push_back(T&& data) {
        should_reallocate();

        buffer_[back_] = std::move(data); // this might be wrong
        back_ = (back_ + 1) % capacity_;
        ++size_;
    }

    T& front() {
        assert(size_ != 0);

        return buffer_[front_];
    }

    T& back() {
        assert(size_ != 0);
        INT_TYPE last = (back_ + (size_ - 1)) % capacity_;
        return buffer_[last];
    }

    void pop() {
        assert(size_ != 0);

        // call the destructor
        buffer_[front_].~T();

        front_ = (front_ + 1) % capacity_;
        --size_;
    }

    INT_TYPE size() const noexcept {
        return size_;
    }

    INT_TYPE empty() const noexcept {
        return size_ == 0;
    }

    T& operator[](INT_TYPE i) {
        assert(i >= 0 && i < size_);

        INT_TYPE index_rolling = (front_ + i) % capacity_;
        return buffer_[index_rolling];
    }

    const T& operator[](INT_TYPE i) const {
        assert(i >= 0 && i < size_);

        INT_TYPE index_rolling = (front_ + i) % capacity_;
        return buffer_[index_rolling];
    }

    // TODO: basic algorithms without using iterators
};
}

// then there's the trivial queue. something more experimental
// C++ semantics are too complicated is there a better way?
namespace nstd {

    // accepts plain old data types only
    template <class T, typename INT_TYPE = int>
    struct queue_trivial {
        static_assert(std::is_fundamental<INT_TYPE>(), "INT_TYPE is not an integer");
        static_assert(std::is_trivial<T>(), "type in this queue is not trivial when it needs to be");

        T* buffer_ = nullptr;
        INT_TYPE front_ = 0;
        INT_TYPE back_ = 0; // back is not inclusive, it is one element after the last element
        INT_TYPE capacity_ = 0;
        INT_TYPE size_ = 0;

        queue_trivial() noexcept {}

        queue_trivial(const queue_trivial<T>& queue) = delete;
        queue_trivial<T>& operator=(const queue_trivial<T>& queue) = delete;
        queue_trivial<T>& operator=(queue_trivial<T>&& type) = delete;

        ~queue_trivial() {
            if (buffer_ == nullptr) return;
            free(buffer_);
        }

        void should_reallocate() noexcept {

            if (capacity_ == size_) {

                if (capacity_ == 0) capacity_ = 2;
                else capacity_ *= 2;
                
                T* buffer_new = (T*)malloc(sizeof(T) * capacity_);
                if (buffer_new == nullptr) abort();

                // copy old buffer into new buffer 
                // dont have to worry about insane copy semantics
                memcpy(buffer_new, buffer_, sizeof(T) * size_);

                // free the old buffer 
                free(buffer_);
                buffer_ = buffer_new;

                front_ = 0;
                back_ = size_;
            }
        }
    public:

        void clear() noexcept {
            front_ = 0;
            back_ = 0;
        }

        void push_back(const T& data) noexcept {
            should_reallocate();

            buffer_[back_] = data;
            back_ = (back_ + 1) % capacity_;
            ++size_;
        }

        template<typename FuncInit>
        T& emplace_back(FuncInit init) noexcept {
            should_reallocate();

            T* data = &buffer_[back_];

            init(*data);

            back_ = (back_ + 1) % capacity_;
            ++size_;
            return *data;
        }

        T& emplace_back() noexcept {
            should_reallocate();

            T* data = &buffer_[back_];

            back_ = (back_ + 1) % capacity_;
            ++size_;
            return *data;
        }

        T& front() noexcept {
            assert(size_ != 0);

            return buffer_[front_];
        }

        T& back() noexcept {
            assert(size_ != 0);
            INT_TYPE last = (back_ + (size_ - 1)) % capacity_;
            return buffer_[last];
        }

        template<typename FuncDeinit>
        void pop(FuncDeinit deinit) noexcept {
            assert(size_ != 0);

            deinit(buffer_[front_]);

            front_ = (front_ + 1) % capacity_;
            --size_;
        }

        void pop() noexcept {
            assert(size_ != 0);

            front_ = (front_ + 1) % capacity_;
            --size_;
        }

        INT_TYPE size() const noexcept {
            return size_;
        }

        INT_TYPE empty() const noexcept {
            return size_ == 0;
        }

        T& operator[](INT_TYPE i) noexcept {
            assert(i >= 0 && i < size_);

            INT_TYPE index_rolling = (front_ + i) % capacity_;
            return buffer_[index_rolling];
        }

        const T& operator[](INT_TYPE i) const noexcept {
            assert(i >= 0 && i < size_);

            INT_TYPE index_rolling = (front_ + i) % capacity_;
            return buffer_[index_rolling];
        }
    };

}
