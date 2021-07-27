//#include "runner_test.h"

#include <numeric>
#include <vector>
#include <string>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <random>
#include <cstdlib>

#include "mutex.h"
#include "log_duration.h"
#include "atomic.h"
#include "test_runner_p.h"
#include "mutex_map.h"

using namespace std;

// Реализуйте шаблон Synchronized<T>.
// Метод GetAccess должен возвращать структуру, в которой есть поле T& ref_to_value.
template <typename T>
class Synchronized {
public:
    explicit Synchronized(T initial = T()) : value_(initial) 
    {}
    struct Access 
    {
        Access( T& initial, mutex* mt) :ref_to_value(initial), mtx(mt)
        { }
        ~Access()
        {
            mtx->unlock();
        }
        

        T& ref_to_value;
        mutex* mtx;
    };

    Access GetAccess()
    {        
        mtx_.lock();
        return Access(value_, &mtx_);
    }
private:
    T value_;
    mutable std::mutex mtx_;

};

//void TestConcurrentUpdate() {
//    Synchronized<string> common_string;
//
//    const size_t add_count = 50000;
//    auto updater = [&common_string, add_count] {
//        for (size_t i = 0; i < add_count; ++i) {
//            auto access = common_string.GetAccess();
//            access.ref_to_value += 'a';
//        }
//    };
//
//    auto f1 = async(updater);
//    auto f2 = async(updater);
//
//    f1.get();
//    f2.get();
//
//    ASSERT_EQUAL(common_string.GetAccess().ref_to_value.size(), 2 * add_count);
//}

vector<int> Consume(Synchronized<deque<int>>& common_queue) {
    vector<int> got;

    for (;;) {
        deque<int> q;

        {
            // Мы специально заключили эти две строчки в операторные скобки, чтобы
            // уменьшить размер критической секции. Поток-потребитель захватывает
            // мьютекс, перемещает всё содержимое общей очереди в свою
            // локальную переменную и отпускает мьютекс. После этого он обрабатывает
            // объекты в очереди за пределами критической секции, позволяя
            // потоку-производителю параллельно помещать в очередь новые объекты.
            //
            // Размер критической секции существенно влияет на быстродействие
            // многопоточных программ.
            auto access = common_queue.GetAccess();
            q = move(access.ref_to_value);
        }

        for (int item : q) {
            if (item > 0) {
                got.push_back(item);
            }
            else {
                return got;
            }
        }
    }
}

void TestProducerConsumer() {
    Synchronized<deque<int>> common_queue;

    auto consumer = async(Consume, ref(common_queue));

    const size_t item_count = 100000;
    for (size_t i = 1; i <= item_count; ++i) {
        common_queue.GetAccess().ref_to_value.push_back(i);
    }
    common_queue.GetAccess().ref_to_value.push_back(-1);

    vector<int> expected(item_count);
    iota(begin(expected), end(expected), 1);
    ASSERT_EQUAL(consumer.get(), expected);
}

int main() {
    //TestRunner tr;
   // RUN_TEST(tr, TestConcurrentUpdate);
   // RUN_TEST(tr, TestProducerConsumer);

    TestRunner tr;
    RUN_TEST(tr, TestConcurrentUpdate);
    RUN_TEST(tr, TestReadAndWrite);
    RUN_TEST(tr, TestSpeedup);

    vector<int> numbers(1'000);

    iota(numbers.begin(), numbers.end(), 0);
    int count = 1;
    {
        
        for (int i = 0; i < count; ++i)
        {
           
            const vector<int> even_numbers =
                CopyIfUnordered(numbers, [](int number) { return number % 2 == 0; });
             for (const int number : even_numbers) {
                 cout << number << " "s;
             }
             cout << endl;
        }
       
    }
   
    /*mt19937 generator;
    const int mother_plans = GeneratePlans(generator);
    const int father_plans = GeneratePlans(generator);
    const int son_plans = GeneratePlans(generator);
    const int daughter_plans = GeneratePlans(generator);
    Account account(1'000'000);
    vector<future<int>> spend_futures;
    for (const int plans : {mother_plans, father_plans, son_plans, daughter_plans}) 
    {
        spend_futures.push_back(async(SpendAll, ref(account), plans));
    }
    for (auto& spend_future : spend_futures)
    {
        cout << "Spent " << spend_future.get() << endl;
    }
    cout << account.GetValue() << endl;*/
    return 0;
}