#include <iostream>
#include <vector>
#include "thread_safe_queue.h"

struct Value : public std::pair<std::string, size_t> {
    using Parent = std::pair<std::string, size_t>;

    Value() = default;

    Value(Value&&) = default;
    Value& operator=(Value&&) = default;

    Value(const Value&) = delete;
    Value& operator=(const Value&) = delete;

    Value(const std::string& s, size_t n)
        : Parent(s, n)
    {}

    Value(std::string&& s, size_t n)
        : Parent(std::move(s), n)
    {}
};

void push(thread_safe_queue<Value>& queue)
{
    for (size_t i = 0; i < 100500; ++i) {
        queue.enqueue(std::move(Value("NUM: " + std::to_string(i), 0)));
    }
}

void print(thread_safe_queue<Value>& queue)
{
    try {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        for (;;) {
            Value s;
            queue.pop(s);
            std::cout << s.first << std::endl;
        }
    } catch (std::exception&) {
        std::cout << "Finished" << std::endl;
    }
}

int main()
{
    const int maxQueSize = 1;
    thread_safe_queue<Value> q(maxQueSize);
    auto printer = std::thread(print, std::ref(q));
    auto pusher1 = std::thread(push, std::ref(q));
    auto pusher2 = std::thread(push, std::ref(q));
    auto pusher3 = std::thread(push, std::ref(q));
    pusher1.join();
    pusher2.join();
    pusher3.join();
    q.shutdown();
    printer.join();
    return 0;
}
