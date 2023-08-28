#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <atomic>

class ThreadBarrier
{
private:
    std::mutex Mtx;
    std::condition_variable Cv;
    std::atomic_int WaitingAmount;
public:
    ThreadBarrier() { WaitingAmount.store(0); }

    void Wait(int amount)
    {
        std::mutex sleepMtx;
        std::unique_lock lk(sleepMtx);

        WaitingAmount.fetch_add(1);

        if (WaitingAmount.compare_exchange_strong(amount, amount - 1))
        {
            while (WaitingAmount.load() != 0) Cv.notify_all();
        }
        else
        {
            Cv.wait(lk);
            WaitingAmount.fetch_sub(1);
        }
    }
};

#include <Windows.h>

int main()
{
    ThreadBarrier barr;

    std::thread th1([&barr]()
        {
            std::cout << "Entered th1" << std::endl;
            barr.Wait(2);
            std::cout << "Start th1" << std::endl;

            Sleep(2000);

            std::cout << "Start step 1 th1" << std::endl;
            barr.Wait(2);
            std::cout << "Stop step 1 th1" << std::endl;
        });
    
    Sleep(1000);

    std::thread th2([&barr]()
        {
            std::cout << "Entered th2" << std::endl;
            barr.Wait(2);
            std::cout << "Start th2" << std::endl;

            Sleep(3000);

            std::cout << "Start step 1 th2" << std::endl;
            barr.Wait(2);
            std::cout << "Stop step 1 th2" << std::endl;
        });

    th1.join();
    th2.join();
}