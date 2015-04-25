#pragma once

#include <ctime>
#include <cstdint>

#include <memory>
#include <unordered_map>

namespace leveldb
{
    class DB;
    class Iterator;
    class Slice;
    class Comparator;
    class Cache;
}

class Storage
{
public:
    class Iterator;

    typedef size_t MetricUid;

    Storage(const std::string& dir, size_t cacheSizeMb = 16);

    MetricUid addMetric(const std::string& name);

    bool put(MetricUid muid,  time_t timestamp, double value);

    Iterator get(MetricUid muid,  time_t from, time_t to);

    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;

    union Key
    {
        struct
        {
            MetricUid muid;
            time_t timestamp;
        };
        char data[sizeof(time_t) + sizeof(MetricUid)];
    };

private:

    void initCfg();
    void initData();

private:

    size_t m_currentIndx;
    std::string m_dir;
    size_t m_cacheSizeMb;

    std::shared_ptr<leveldb::Cache> m_dataCache;
    std::shared_ptr<leveldb::DB> m_cfg;
    std::shared_ptr<leveldb::DB> m_data;
    std::unordered_map<std::string, MetricUid> m_metric2indx;
};

class Storage::Iterator
{
public:
    typedef std::tuple<time_t, double> Value;
    typedef std::shared_ptr<leveldb::Iterator> IteratorPrivate;

    Iterator();
    Iterator(const IteratorPrivate& iter, const Key& limit);

    bool valid() const;
    Value value() const;

    void next();

private:
    IteratorPrivate m_iter;
    Key m_limit;
};
