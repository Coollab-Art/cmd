#pragma once

#include <list>

namespace cmd::internal {

template<typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t max_size)
        : _max_size{max_size}
    {
    }

    void push_back(const T& t)
    {
        push_back_impl(t);
    }

    void push_back(T&& t)
    {
        push_back_impl(std::move(t));
    }

    auto max_size() const -> size_t { return _max_size; }
    void set_max_size(size_t new_max_size)
    {
        _max_size = new_max_size;
        shrink_to_max_size();
    }

    auto underlying_container() -> std::list<T>& { return _container; }

private:
    template<typename Tref>
    void push_back_impl(Tref&& t)
    {
        _container.push_back(std::forward<Tref>(t));
        shrink_to_max_size();
    }

    void shrink_to_max_size()
    {
        while (_container.size() > _max_size) {
            _container.pop_front();
        }
    }

private:
    std::list<T>
           _container;
    size_t _max_size;
};

} // namespace cmd::internal
