#pragma once
#include <algorithm>
#include <execution>
#include <mutex>
#include <vector>
#include <iostream>
#include <numeric>
#include <future>
#include "log_duration.h"

using namespace std;


template <typename Container, typename Predicate>
vector<typename Container::value_type> CopyIfUnordered1(const Container& container, Predicate predicate)
{
    LOG_DURATION("main"s);
    mutex m;
    vector<typename Container::value_type> result;
    result.reserve(container.size());
    //int indx=0;
    for_each(execution::par, container.begin(), container.end(),
        [&predicate, &m, &result](auto& value)
        {
            int indx = 0;
            if (predicate(value)) {
                {

                    lock_guard<mutex> g(m);
                    result.resize(result.size() + 1);
                    indx = result.size() - 1;
                }     //lock_guard<mutex>g(m);
                result[indx] = move(value);
                //++indx;
                 //result.push_back(value);
            }
            // result.resize(indx+1);
        });

    return result;
}

template <typename Container, typename Predicate>
vector<typename Container::value_type> CopyIfUnordered2(const Container& container, Predicate predicate) 
{
    LOG_DURATION("other"s);
    vector<typename Container::value_type> result;
    result.reserve(container.size());
    mutex result_mutex;
    for_each(
        execution::par,
        container.begin(), container.end(),
        [predicate, &result_mutex, &result](const auto& value)
        {
            if (predicate(value)) 
            {
                typename Container::value_type* destination;
                {
                    lock_guard guard(result_mutex);
                    destination = &result.emplace_back();
                }
                *destination = value;
            }
        }
    );
    return result;
}

//template <typename Container, typename Predicate>
//vector<typename Container::value_type> CopyIfUnordered(const Container& container, Predicate predicate)
//{
//    LOG_DURATION("push_back"s);
//    vector<typename Container::value_type> result;
//    for (const auto& value : container) 
//    {
//        if (predicate(value)) 
//        {
//            result.push_back(value);
//        }
//    }
//    return result;
//}

template <typename Container, typename Predicate>
vector<typename Container::value_type> CopyIfUnordered3(const Container& container, Predicate predicate)
{
    LOG_DURATION("main main"s);
    vector<typename Container::value_type> result;
    size_t distance = container.size();
    const auto begin_ = container.begin();
    const auto end_ = container.end();
    const auto it_left = next(container.begin(), distance / 2);

    mutex m;

    auto thr1 = async([&]
        {
            for (auto it = begin_; it != it_left; ++it)
            {
                if (predicate(*it))
                {
                    lock_guard g(m);
                    result.push_back(*it);
                }
            }
        });
    
    auto thr2 = async([&]
        {
            for (auto it = it_left; it != end_; ++it)
            {

                if (predicate(*it))
                {
                    lock_guard g(m);
                    result.push_back(*it);
                }
            }
        });
    thr1.get();
    thr2.get();
    return result;
}