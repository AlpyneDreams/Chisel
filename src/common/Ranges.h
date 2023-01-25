#pragma once

#include <algorithm>
#include <utility>
#include <iterator>

namespace engine
{
    /** Simple iterable range on interval [first, last)
     * Similar to std::ranges::subrange. Can be used with iterator
     * pairs returned by functions like std::multimap::equal_range
     */
    template <std::input_or_output_iterator Iterator>
    struct Subrange
    {
        Iterator first, last;

        Subrange(std::pair<Iterator, Iterator> pair) : first(pair.first), last(pair.second) {}
        Subrange(Iterator first, Iterator last) : first(first), last(last) {}
        
        Iterator begin() const { return first; }
        Iterator end() const { return last; }
    };

    namespace util
    {
        constexpr bool contains(auto&& c, auto x) {
            using namespace std;
            return find(begin(c), end(c), x) != end(c);
        }
    }
}