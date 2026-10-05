//
// Created by yuzhang on 2026/10/5.
//
#pragma once
#include <chrono>
#include <thread>

namespace util
{
    template<typename Key, typename Value>
    class TtlCache
    {
    private:
        struct Entry
        {
            Value value;
            std::chrono::steady_clock::time_point expire_at;
        };

        std::mutex mutex_;
        std::unordered_map<Key, Entry> data_;
        std::chrono::seconds cleanup_interval_;
        std::thread cleanup_thread_;
        bool stopped_;
        std::condition_variable cv_;

        bool is_expired(const Entry &entry)
        {
            return std::chrono::steady_clock::now() >= entry.expire_at;
        }

        void cleanup_loop()
        {
            std::unique_lock<std::mutex> lock(mutex_);

            while (!stopped_)
            {
                cv_.wait_for(lock, cleanup_interval_, [this]
                {
                   return stopped_;
                });
                if (stopped_)
                {
                    break;
                }
                cleanup_expired();
            }
        }

        void cleanup_expired()
        {
            const auto now = std::chrono::steady_clock::now();
            for (auto it = data_.begin(); it != data_.end();)
            {
                if (now >= it->second.expire_at)
                {
                    it = data_.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    public:
        explicit TtlCache(std::chrono::seconds cleanup_interval = std::chrono::seconds(60)) : cleanup_interval_(cleanup_interval), cleanup_thread_([this]{cleanup_loop();}){}

        ~TtlCache()
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                stopped_ = true;
            }
            // 唤醒等待在cv上的线程
            cv_.notify_all();

            if (cleanup_thread_.joinable())
            {
                cleanup_thread_.join();
            }
        }

        TtlCache(const TtlCache&) = delete;
        TtlCache &operator=(const TtlCache&) = delete;

        void put(const Key &key, const Value &value, std::chrono::seconds ttl)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            data_[key] = Entry
            {
                value,
                std::chrono::steady_clock::now() + ttl
            };
        }

        std::optional<Value> get(const Key &key)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = data_.find(key);
            if (it == data_.end())
            {
                return std::nullopt;
            }
            if (is_expired(it->second))
            {
                data_.erase(key);
                return std::nullopt;
            }
            return it->second.value;
        }

        bool contains(const Key &key)
        {
            return get(key).has_value();
        }

        void remove(const Key &key)
        {
            std::lock_guard<std::mutex> lock_guard(mutex_);
            data_.erase(key);
        }

    };
}

