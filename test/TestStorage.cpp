#include <iostream>

#include "gtest/gtest.h"
#include "Storage.h"

TEST(Storage, sequentialAccess)
{
    Storage storage("./test");

    const auto muid = storage.addMetric("foo");
    const double value = 15;

    const time_t start = 1400000000;
    const time_t finish = 1400030000;
    const size_t step = 300;

    for(time_t i = start; i<finish; i+=step)
    {
        storage.put(muid, i, value);
    }

    const time_t end = 1400021000;
    auto iter =  storage.get(muid,start, end);
    time_t i = start;
    for(;iter.valid();iter.next())
    {
        time_t timestamp = 0;
        double val = 0;
        std::tie(timestamp, val) = iter.value();
        ASSERT_EQ(i, timestamp);
        ASSERT_DOUBLE_EQ(value, val);
        i+=300;
    }
    ASSERT_EQ(1400021000, i);
}
