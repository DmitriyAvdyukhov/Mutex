#pragma once
#include <atomic>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <random>
#include <vector>

using namespace std;

class Account {
public:
    explicit Account(int value) : value_(value) {}
    int GetValue() const { return value_; }
    bool TrySpendOne() {
        if (value_.fetch_sub(1) <= 0) {
            ++value_;
            return false;
        }
        else {
            return true;
        }
    }
private:
    atomic_int value_;
};

int SpendAll(Account& account, int plans) {
    int total_spent = 0;
    for (int i = 0; i < plans; ++i) {
        if (account.TrySpendOne()) {
            ++total_spent;
        }
        else {
            break;
        }
    }
    return total_spent;
}

int GeneratePlans(mt19937& generator, int max_count = 1'000'000) {
    return uniform_int_distribution(0, max_count)(generator);
}

template <typename Container, typename Predicate>
vector<typename Container::value_type> CopyIfUnordered(const Container& container, Predicate predicate)
{   
    vector<typename Container::value_type> result(container.size());   
    atomic_int size = 0;    
    for_each(
        execution::par,
        container.begin(), container.end(),
        [predicate,  &result, &size]( const auto& value)
        {             
            if (predicate(value)) 
            {  
                result[size++] = value;                                                 
            }           
        }   
    );   
    result.resize(size);
    return result;
}

