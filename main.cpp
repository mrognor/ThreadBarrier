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
    std::atomic_bool IsEnded;
public:
    ThreadBarrier() 
    { 
        WaitingAmount.store(0);
        IsEnded.store(true);
    }

    void Wait(int amount)
    {
        std::mutex mtx;
        std::unique_lock lk(mtx);

        WaitingAmount.fetch_add(1);

        if (WaitingAmount.compare_exchange_strong(amount, amount - 1))
        {
            IsEnded.store(false);
            while (WaitingAmount.load() != 0) Cv.notify_all();
            WaitingAmount.store(0);
            IsEnded.store(true);
        }
        else
        {
            Cv.wait(lk, [this]() { return IsEnded.load() == false; });
            WaitingAmount.fetch_sub(1);

            while (!IsEnded.load()) {}
        }
    }
};

int main()
{
    ThreadBarrier barrier;
    std::atomic_int64_t counter(0);

    std::thread th1([&]()
    {
        for (std::size_t i = 0; i < 10000000; ++i)
        {
            counter.fetch_add(1);
            barrier.Wait(2);
        }

        std::cout << "Th: " << counter << std::endl;
    });

    for (std::size_t i = 0; i < 10000000; ++i)
    {
        counter.fetch_sub(1);
        barrier.Wait(2);
    }
    std::cout << "Main: " << counter << std::endl;

    th1.join();
}
