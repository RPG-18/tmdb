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
            const Storage::Key* keyA =
                    reinterpret_cast<const Storage::Key*>(dataA);
            const Storage::Key* keyB =
                    reinterpret_cast<const Storage::Key*>(dataB);
            if (keyA->muid < keyB->muid)
            {
                return -1;
            }
            else if (keyA->muid > keyB->muid)
            {
                return 1;
            }

            if (keyA->timestamp < keyB->timestamp)
            {
                return -1;
            }
            else if (keyA->timestamp > keyB->timestamp)
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
        m_uid(nullptr),
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

    initUID();
    initData();
}

void Storage::initUID()
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
    m_uid.reset(cfg);

    std::unique_ptr<leveldb::Iterator> it(
            m_uid->NewIterator(leveldb::ReadOptions()));

    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        const size_t* index = reinterpret_cast<const size_t*>(it->key().data());
        m_metric2uid[it->value().ToString()] = *index;
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

Storage::MetricUid Storage::addMetric(const std::string& name)
{
    auto result = m_metric2uid.find(name);
    if (result != m_metric2uid.end())
    {
        return result->second;
    }
    ++m_currentIndx;
    m_metric2uid[name] = m_currentIndx;
    const auto s = m_uid->Put(WriteOptions(),
            Slice(reinterpret_cast<const char*>(&m_currentIndx), sizeof(m_currentIndx)),
            name);

    if (!s.ok())
    {
        LOG(ERROR)<<"Error put "<<s.ToString();
    }

    return m_currentIndx;
}

bool Storage::put(MetricUid muid, time_t timestamp, double value)
{
    const Key key = {muid, timestamp};

    const auto s = m_data->Put(WriteOptions(),
            Slice(reinterpret_cast<const char*>(&key), sizeof(key)),
            Slice(reinterpret_cast<char*>(&value), sizeof(value)));

    if (!s.ok())
    {
        LOG(ERROR)<<"Error put "<<s.ToString();
    }

    return s.ok();
}

Storage::Iterator Storage::get(MetricUid muid, time_t from, time_t to)
{
    const Key begin = {muid, from};
    const Key end = { muid, to };

    Storage::Iterator::IteratorPrivate iter(m_data->NewIterator(ReadOptions()));
    iter->Seek(Slice(reinterpret_cast<const char*>(&begin),
                     sizeof(begin)));
    return Storage::Iterator(iter, end);
}

Storage::Iterator::Iterator():
        m_iter(nullptr)
{
    memset(&m_limit, 0, sizeof(m_limit));
}


Storage::Iterator::Iterator(const IteratorPrivate& iter, const Key& limit) :
        m_iter(iter),
        m_limit(limit)
{
}

bool Storage::Iterator::valid() const
{
    if(!m_iter)
    {
        return false;
    }

    const Slice right(reinterpret_cast<const char*>(&m_limit),
                      sizeof(m_limit));

    return m_iter->Valid() &&
           (GLOBAL_COMPORATOR.Compare(m_iter->key(),right) < 0);
}

Storage::Iterator::Value Storage::Iterator::value() const
{
    if(!m_iter)
    {
        return Value(0,0);
    }

    const Key* data =reinterpret_cast<const Key*>(m_iter->key().data());
    double val = *reinterpret_cast<const double*>(m_iter->value().data());
    return Value(data->timestamp, val);
}

void Storage::Iterator::next()
{
    if(m_iter)
    {
        m_iter->Next();
    }
}
