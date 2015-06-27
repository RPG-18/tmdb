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

/*!
 * Хранилище метрик
 */
class Storage
{
public:
    class Iterator;
    typedef size_t MetricUid;

    /*!
     * Представление ключа
     */
    struct Key
    {
        MetricUid muid;   //!< uid метрики
        time_t timestamp; //!< время
    };

    /*!
     * Конструктор
     * @param dir каталог для размещения базы данных и базы метрик
     * @param cacheSizeMb размер блока кеша
     */
    Storage(const std::string& dir, size_t cacheSizeMb = 16);

    /*!
     * @brief Добавление метрики.
     * @param name имя метрики
     * @return уникальный идентификатор метрики
     *
     * Добавляет метрику в базу UID'ов и возвращает UID метрики.
     * Если метрика уже была добавлена, то возращает UID метрики
     */
    MetricUid addMetric(const std::string& name);

    /*!
     * Записать значение
     * @param muid идентификатор метрики
     * @param timestamp временная точка
     * @param value значение
     * @return true если нет ошибок
     */
    bool put(MetricUid muid, time_t timestamp, double value);

    /*!
     * Получить итератор для интервала значений метрики
     * @param muid идентификатор метрики
     * @param from начало интервала
     * @param to конец интервала
     * @return итератор
     */
    Iterator get(MetricUid muid, time_t from, time_t to);

    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;

private:

    /*!
     * Инициализация базы uid метрик
     */
    void initUID();

    /*!
     * Инициализация данных
     */
    void initData();

private:

    /*!
     * Текущий индекс для UID
     */
    MetricUid m_currentIndx;

    /*!
     * Базавый каталог
     */
    std::string m_dir;

    /*!
     * Размер блока кеша
     */
    size_t m_cacheSizeMb;

    /*!
     * Кеш для данных
     */
    std::shared_ptr<leveldb::Cache> m_dataCache;

    /*!
     * База UID'ов
     */
    std::shared_ptr<leveldb::DB> m_uid;

    /*!
     * База измерений
     */
    std::shared_ptr<leveldb::DB> m_data;

    /*!
     * Мэп метрика -> uid
     */
    std::unordered_map<std::string, MetricUid> m_metric2uid;
};

/*!
 * Итератор для обхода последовательности данных
 */
class Storage::Iterator
{
public:
    typedef std::tuple<time_t, double> Value;
    typedef std::shared_ptr<leveldb::Iterator> IteratorPrivate;

    Iterator();

    Iterator(const IteratorPrivate& iter, const Key& limit);

    /*!
     * Проверка итератора на валидность
     * @return true если итератор валиден
     */
    bool valid() const;

    /*!
     * Полуяить значение
     * @return кортеж <время, значение>
     */
    Value value() const;

    /*!
     * Переход к следующему элементу
     */
    void next();

private:

    IteratorPrivate m_iter; //!< итератор LevelDB
    Key m_limit; //!< ключ для ограничения последовательности справа
};
