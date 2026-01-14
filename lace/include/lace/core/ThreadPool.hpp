//
//  Copyright (c) 2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_THREAD_POOL_H_
#define LOVELACE_THREAD_POOL_H_

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

namespace lace {

using Job = std::function<void()>;

class ThreadPool final {
    std::vector<std::jthread> m_threads = {};
    std::queue<Job> m_jobs = {};

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::condition_variable m_done;

    uint32_t m_pending = 0;

public:
    ThreadPool(uint32_t count) {
        m_threads.reserve(count);

        for (uint32_t i = 0; i < count; ++i) {
            m_threads.emplace_back([this](std::stop_token token) {
                worker(token);
            });
        }
    }

    ~ThreadPool() {
        request_stop();
    }

    ThreadPool(const ThreadPool&) = delete;
    void operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) noexcept = delete;
    void operator=(ThreadPool&&) noexcept = delete;

    void push(Job job) {
        {
            std::lock_guard guard(m_mutex);
            m_jobs.push(std::move(job));
            ++m_pending;
        }

        m_cv.notify_one();
    }

    void wait() {
        std::unique_lock lock(m_mutex);
        m_done.wait(lock, [&] {
            return m_pending == 0;
        });
    }

    uint32_t get_pending() const { return m_pending; }

private:
    void worker(std::stop_token token) {
        while (!token.stop_requested()) {
            Job job;

            {
                std::unique_lock lock(m_mutex);
                m_cv.wait(lock, [&] {
                    return token.stop_requested() || !m_jobs.empty();
                });

                if (token.stop_requested())
                    return;

                job = std::move(m_jobs.front());
                m_jobs.pop();
            }

            job();

            {
                std::lock_guard lock(m_mutex);
                --m_pending;
                
                if (m_pending == 0)
                    m_done.notify_all();
            }
        }
    }

    void request_stop() {
        for (std::jthread& thr : m_threads)
            thr.request_stop();

        m_cv.notify_all();
    }
};


} // namespace lace

#endif // LOVELACE_THREAD_POOL_H_
