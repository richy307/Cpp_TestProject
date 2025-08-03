#include <iostream>
#include <thread>
// using namespace std;



// Test Hello World 
void testHelloWorld() 
{
    std::cout << "Hello, World!" << std::endl;
}

class Foo 
{
public:
    void member_function() 
    {
        std::cout << __func__ << " this_thread_id: " << std::this_thread::get_id() << std::endl;
    }

    void member_function2(const std::string& str) 
    {
        std::cout << __func__ << " this_thread_id: " << std::this_thread::get_id() << " str: " << str << std::endl;
    }
};

void testThread() 
{
    // 1.取得當前執行緒的 ID 
    std::cout << __func__ << " this_thread_id: " << std::this_thread::get_id() << std::endl;

    
    // 2.目前硬體上可用的並行執行緒數量 // 例如設定 thread pool 的大小或是做任務分配。
    // 如果返回值為 0，表示無法確定硬體的並行執行緒數量。
    const unsigned int num_threads = std::thread::hardware_concurrency();
    // const auto num_threads = std::thread::hardware_concurrency();
    std::cout << __func__ << " hardware_concurrency: " << num_threads << std::endl;

    
    // 3.建立一個新的執行緒，並在其中執行一個 lambda 函數 (member function, function pointer, or callable object)
    std::thread t1([]() 
    {
        std::cout << "Hello from thread t1!" << std::endl;

        std::cout << __func__ << " t1_this_thread_id: " << std::this_thread::get_id() << std::endl;

        // 模擬一些工作
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    
    std::cout << __func__ << " t1_thread_id: " << t1.get_id() << std::endl;

    if (t1.joinable()) // 檢查 t1 是否可 joinable
    {
        std::cout << __func__ << " t1 is joinable" << std::endl;

        t1.join(); // 等待 t1 執行完成

        // t1.detach(); // 分離 t1，讓它在背景繼續執行
        // 注意：在分離後，t1 的資源會被釋放
        // 但如果 t1 還在執行，則可能會導致未定義行為
        // 因此在分離之前，應確保 t1 已經完成或不再需要 join。

        std::cout << __func__ << " t1 has finished" << std::endl;
    } 
    else 
    {
        std::cout << __func__ << " t1 is not joinable" << std::endl;
    }
    

    // 4.建立一個新的執行緒，並在其中執行 Foo::member_function
    Foo foo;
    std::thread t2(&Foo::member_function, &foo);
    std::cout << __func__ << " t2 has finished" << std::endl;


    // 5.使用 std::bind 將成員函數與物件實例綁定
    Foo foo2;
    std::thread t3(std::bind(&Foo::member_function, foo2)); // 注意：這裡傳遞的是 foo2 的副本，可能會導致未定義行為
    std::cout << __func__ << " t3 has finished" << std::endl;

    std::thread t4(std::bind(&Foo::member_function2, &foo2, "Hello from t4")); // 注意：這裡傳遞的是 foo2 的地址，這是正確的做法
    std::cout << __func__ << " t4 has finished" << std::endl;


    // 6.使用 std::thread 的移動構造函數
    std::thread t5([]() {
        std::cout << "Hello from thread t5!" << std::endl;
        std::cout << __func__ << " t5 has finished" << std::endl;
    });

    // std::thread t6 = t5; // 複製 t5 到 t6
    // 注意：這裡會導致錯誤，因為 std::thread 的 "複製構造函數" 被刪除。
    // std::thread t6(t5); // 這樣會導致編譯錯誤，因為 std::thread 不允許複製。

    // 正確的做法是使用"移動構造函數"。
    // 這樣會將 t2 的資源轉移到 t5，t2 將不再可用。

    std::cout << __func__ << " before t5 thread_id: " << t5.get_id() << std::endl; // 例如：0x16ff47000
    std::thread t6 = std::move(t5); // 移動 t5 到 t6
    std::cout << __func__ << " after t5 thread_id: " << t5.get_id() << std::endl; // 0x0
    std::cout << __func__ << " t6 thread_id: " << t6.get_id() << std::endl; // 例如：0x16ff47000

    // std::thread t6(std::move(t5)); // 這樣會導致錯誤，因為 t5 已經被移動了。
    // 注意：
    // 1. std::thread 的複製構造函數被刪除，因此不能直接複製 std::thread 對象。
    // 2. 使用 std::move 可以將 t5 的資源轉移到 t6，但這樣會使得 t5 不再可用。
    // 3. 在移動後，t5 將不再可用，不能再對其進行 join 或 detach 操作。
    // 4. 如果需要在多個執行緒之間共享資源，可以考慮使用 std::shared_ptr 或 std::unique_ptr 等智能指針，來管理資源的生命週期。
    // 5. 如果需要在多個執行緒之間共享 std::thread 對象，可以考慮使用 std::jthread 或 std::shared_future 等類型，來管理執行緒的生命週期。

    // 不能用 0x0 來判斷 t5 物件是不是管理空的 thread
    // 因為 不同的平台和編譯器可能會有不同的實現。
    // 在 windows 上，get_id() 會返回一個 std::thread::id，是一個 class。
    // 必須要用 thread::id() 的 預設建構子來判斷 t5 是否是空的 thread。
    
    if (std::thread::id() == t5.get_id()) 
    {
        std::cout << __func__ << " t5 is no thread" << std::endl;
    } 
    else 
    {
        std::cout << __func__ << " t5 is has thread" << std::endl;
    }


    // 7.使用 std::jthread (C++20)
    // std::jthread 是 C++20 引入的類型，它自動管理執行緒的生命週期，並提供了停止令牌的功能。
    // 注意：這需要 C++20 支持。
    // 這裡使用 C++17，所以不會使用 std::jthread。
    // std::jthread t7([]() {
    //     std::cout << "Hello from jthread t7!" << std::endl;
    // });


    // 注意：這裡 t7 是一個空的 std::thread 對象。
    std::thread t7; // 預設建構子，t7 不會管理任何執行緒。
    std::cout << __func__ << " t7 thread_id: " << t7.get_id() << std::endl; // 例如：0x0

    // 如果需要使用 t7，必須先給它分配一個執行緒。
    // std::thread 不能直接使用預設建構子來創建一個空的執行緒。
    t7 = std::thread([]() {
        std::cout << "Hello from thread t7!" << std::endl;
        std::cout << __func__ << " t7 has finished" << std::endl;
    });
    std::cout << __func__ << " t7 thread_id: " << t7.get_id() << std::endl; // 例如：0x0

    // t7.native_handle(); // 獲取底層的原生執行緒句柄
    // pthread_create_suspended_np(t7.native_handle(), nullptr, [](void*) -> void* {
    //     std::cout << "Hello from suspended thread t7!" << std::endl;
    //     return nullptr;
    // }, nullptr); // 創建一個暫停的執行緒
    

    t2.join(); // 等待 t2 執行完成
    // t2.join(); // 等待 t2 執行完成 // 第二次 join 會導致錯誤，因為 t2 已經被 join 過了。
    t3.join(); // 等待 t3 執行完成
    t4.join(); // 等待 t4 執行完成
    t6.join(); // 等待 t6 執行完成
    t7.join(); // 等待 t7 執行完成
}

int main() 
{

    // testHelloWorld();

    testThread();


    return 0;
}