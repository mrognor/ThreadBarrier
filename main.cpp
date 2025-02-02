#include <fstream>
#include <cstdint>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <Windows.h>

class ThreadBarrier
{
private:
    std::condition_variable Cv;
    std::atomic_int WaitingAmount;
    std::atomic_bool IsEnded = false;
public:
    ThreadBarrier() { WaitingAmount.store(0); }

    void Wait(int amount)
    {
        std::mutex sleepMtx;
        std::unique_lock lk(sleepMtx);

        // std::cout << std::this_thread::get_id() << std::endl;

        WaitingAmount.fetch_add(1);

        if (WaitingAmount.compare_exchange_strong(amount, amount - 1))
        {
            IsEnded = false;
            while (WaitingAmount.load() != 0) Cv.notify_all();
            WaitingAmount.store(0);
            IsEnded = true;
        }
        else
        {
            Cv.wait(lk); // Handle spurious wake up
            WaitingAmount.fetch_sub(1);

            while (!IsEnded) {}
        }
    }
};

int main()
{
    ThreadBarrier barrier;
    std::atomic_int64_t counter(0);

    std::thread th1([&]()
    {
        for (std::size_t i = 0; i < 1000000; ++i)
        {
            counter.fetch_add(1);
            barrier.Wait(2);
        }

        std::cout << "Th: " << counter << std::endl;
    });

    for (std::size_t i = 0; i < 1000000; ++i)
    {
        counter.fetch_sub(1);
        // Sleep(100);
        barrier.Wait(2);
    }
    std::cout << "Main: " << counter << std::endl;

    th1.join();
}
