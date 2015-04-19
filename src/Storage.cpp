#include <glog/logging.h>

#include <leveldb/db.h>
#include <leveldb/cache.h>
#include <leveldb/comparator.h>

#include <boost/filesystem.hpp>

#include "Storage.h"

using namespace leveldb;
using namespace boost::filesystem;
namespace
{
    class TimeMeasurementComporator: public leveldb::Comparator
    {
    public:
        int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const
        {
            const char* dataA = a.data();
            const char* dataB = b.data();
            const Storage::MetricUid* uidA =
                    reinterpret_cast<const Storage::MetricUid*>(dataA);
            const Storage::MetricUid* uidB =
                    reinterpret_cast<const Storage::MetricUid*>(dataB);
            if (*uidA < *uidB)
            {
                return -1;
            }
            else if (*uidA > *uidB)
            {
                return 1;
            }

            const time_t* timeA = reinterpret_cast<const time_t*>(dataA
                    + sizeof(Storage::MetricUid));
            const time_t* timeB = reinterpret_cast<const time_t*>(dataB
                    + sizeof(Storage::MetricUid));

            if (*timeA < *timeB)
            {
                return -1;
            }
            else if (*timeA > *timeB)
            {
                return 1;
            }

            return 0;
        }

        // Ignore the following methods for now:
        const char* Name() const
        {
            return "TimeMeasurementComporator";
        }
        void FindShortestSeparator(std::string*, const leveldb::Slice&) const
        {
        }
        void FindShortSuccessor(std::string*) const
        {
        }
    };
    TimeMeasurementComporator GLOBAL_COMPORATOR;
}

Storage::Storage(const std::string& dir, size_t cacheSizeMb) :
        m_currentIndx(0),
        m_dir(dir),
        m_cacheSizeMb(cacheSizeMb),
        m_cfg(nullptr),
        m_data(nullptr)
{
    path p(dir);
    if(!exists(p))
    {
        boost::system::error_code ec;
        if(!create_directory(p,ec))
        {
            LOG(ERROR)<<"Failed to create directory: "<<ec.message();
        }
    }

    initCfg();
    initData();
}

void Storage::initCfg()
{
    Options options;
    options.create_if_missing = true;

    DB* cfg;
    Status status = DB::Open(options, m_dir + "/conf", &cfg);
    if (!status.ok())
    {
        LOG(ERROR)<<"Error opening database "<<status.ToString();
        exit(1);
    }
    m_cfg.reset(cfg);

    std::unique_ptr<leveldb::Iterator> it(
            m_cfg->NewIterator(leveldb::ReadOptions()));

    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        const size_t* index = reinterpret_cast<const size_t*>(it->key().data());
        m_metric2indx[it->value().ToString()] = *index;
        m_currentIndx = *index;
    }
}

void Storage::initData()
{
    DB* data;
    Options options;
    options.create_if_missing = true;
    options.compression = kNoCompression;
    options.comparator = &GLOBAL_COMPORATOR;

    if (m_cacheSizeMb)
    {
        options.block_cache = leveldb::NewLRUCache(m_cacheSizeMb * 1048576);
        m_dataCache.reset(options.block_cache);
    }

    Status status = DB::Open(options, m_dir + "/data", &data);
    if (!status.ok())
    {
        LOG(ERROR)<<"Error opening database "<<status.ToString();
        exit(1);
    }
    m_data.reset(data);
}

size_t Storage::addMetric(const std::string& name)
{
    auto result = m_metric2indx.find(name);
    if (result != m_metric2indx.end())
    {
        return result->second;
    }
    ++m_currentIndx;
    m_metric2indx[name] = m_currentIndx;
    return m_currentIndx;
}

bool Storage::put(MetricUid muid, time_t timestamp, double value)
{
    char key[KEY_SIZE];
    packKey(key, muid, timestamp);

    const auto s = m_data->Put(WriteOptions(), Slice(key, KEY_SIZE),
            Slice(reinterpret_cast<char*>(&value), sizeof(value)));

    if (!s.ok())
    {
        LOG(ERROR)<<"Error put "<<s.ToString();
    }

    return s.ok();
}

Storage::Iterator Storage::get(MetricUid muid, time_t from, time_t to)
{
    char begin[KEY_SIZE];
    packKey(begin, muid, from);

    char end[KEY_SIZE];
    packKey(end, muid, to);

    Storage::Iterator::IteratorPrivate iter(m_data->NewIterator(ReadOptions()));
    iter->Seek(Slice(begin, KEY_SIZE));
    return Storage::Iterator(iter, end);
}

Storage::Iterator::Iterator():
        m_iter(nullptr)
{
    memset(m_limit, 0, sizeof(m_limit));
}


Storage::Iterator::Iterator(const IteratorPrivate& iter, const char limit[]) :
        m_iter(iter)
{
    memcpy(m_limit, limit, Storage::KEY_SIZE);
}

bool Storage::Iterator::valid() const
{
    if(!m_iter)
    {
        return false;
    }

    return m_iter->Valid()
            && (GLOBAL_COMPORATOR.Compare(m_iter->key(),
                    Slice(m_limit, Storage::KEY_SIZE)) < 0);
}

Storage::Iterator::Value Storage::Iterator::value() const
{
    if(!m_iter)
    {
        return Value(0,0);
    }

    const char* data = m_iter->key().data();
    time_t time = 0;
    memcpy(&time, data + sizeof(Storage::MetricUid), sizeof(time));
    double val = *reinterpret_cast<const double*>(m_iter->value().data());
    return Value(time, val);
}

void Storage::Iterator::next()
{
    if(m_iter)
    {
        m_iter->Next();
    }
}
