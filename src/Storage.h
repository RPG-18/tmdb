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

    static constexpr size_t KEY_SIZE = sizeof(MetricUid) + sizeof(time_t);

    Storage(const std::string& dir, size_t cacheSizeMb = 16);

    MetricUid addMetric(const std::string& name);

    bool put(MetricUid muid,  time_t timestamp, double value);

    Iterator get(MetricUid muid,  time_t from, time_t to);

    void packKey(char* key, MetricUid muid, time_t timestamp)
    {
        memset(key, 0, KEY_SIZE);
        memcpy(key, &muid, sizeof(muid));
        memcpy(key + sizeof(muid), &timestamp, sizeof(time_t));
    }

    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;
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
    Iterator(const IteratorPrivate& iter, const char limit[]);

    bool valid() const;
    Value value() const;

    void next();

private:
    IteratorPrivate m_iter;
    char m_limit[KEY_SIZE];
};
