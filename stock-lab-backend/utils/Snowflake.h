//
// Created by yuzhang on 2026/10/5.
//

#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace util
{
    class Snowflake
    {
    public:
        Snowflake(int64_t worker_id, int64_t datacenter_id)
            : worker_id_(worker_id), datacenter_id_(datacenter_id)
        {
            if (worker_id < 0 || worker_id > k_max_worker_id)
            {
                throw std::invalid_argument("worker_id out of range");
            }
            if (datacenter_id < 0 || datacenter_id > k_max_datacenter_id)
            {
                throw std::invalid_argument("datacenter_id out of range");
            }
        }

        int64_t next_id()
        {
            std::lock_guard<std::mutex> lock(mutex_);

            int64_t timestamp = current_millis();
            if (timestamp < last_timestamp_)
            {
                throw std::runtime_error("clock moved backwards");
            }

            if (timestamp == last_timestamp_)
            {
                sequence_ = (sequence_ + 1) & k_sequence_mask;
                if (sequence_ == 0)
                {
                    timestamp = wait_next_millis(last_timestamp_);
                }
            }
            else
            {
                sequence_ = 0;
            }

            last_timestamp_ = timestamp;

            return ((timestamp - k_epoch) << k_timestamp_left_shift)
                | (datacenter_id_ << k_datacenter_id_shift)
                | (worker_id_ << k_worker_id_shift)
                | sequence_;
        }

    private:
        static constexpr int64_t k_epoch = 1288834974657LL;
        static constexpr int64_t k_worker_id_bits = 5;
        static constexpr int64_t k_datacenter_id_bits = 5;
        static constexpr int64_t k_sequence_bits = 12;

        static constexpr int64_t k_max_worker_id = (1LL << k_worker_id_bits) - 1;
        static constexpr int64_t k_max_datacenter_id = (1LL << k_datacenter_id_bits) - 1;
        static constexpr int64_t k_sequence_mask = (1LL << k_sequence_bits) - 1;

        static constexpr int64_t k_worker_id_shift = k_sequence_bits;
        static constexpr int64_t k_datacenter_id_shift = k_sequence_bits + k_worker_id_bits;
        static constexpr int64_t k_timestamp_left_shift =
            k_sequence_bits + k_worker_id_bits + k_datacenter_id_bits;

        static int64_t current_millis()
        {
            using namespace std::chrono;
            return duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()).count();
        }

        static int64_t wait_next_millis(int64_t last_timestamp)
        {
            int64_t timestamp = current_millis();
            while (timestamp <= last_timestamp)
            {
                std::this_thread::yield();
                timestamp = current_millis();
            }
            return timestamp;
        }

        int64_t worker_id_;
        int64_t datacenter_id_;
        int64_t sequence_{0};
        int64_t last_timestamp_{-1};
        std::mutex mutex_;
    };
}
