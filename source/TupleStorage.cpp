#include "libs.h"
using namespace std;

// for hashing tuples
struct hash_tuple
{
    template <class T1, class T2>
    size_t operator()(const tuple<T1, T2> &tup) const
    {
        auto h1 = hash<T1>{}(get<0>(tup));
        auto h2 = hash<T2>{}(get<1>(tup));
        return h1 ^ h2;
    }
};

namespace std
{
    template <typename T>
    struct hash<std::tuple<T, T>>
    {
        size_t operator()(const std::tuple<T, T> &t) const
        {
            size_t h = 0;
            std::hash<T> hash_fn;
            h ^= hash_fn(std::get<0>(t)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash_fn(std::get<1>(t)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
}

// provided key return count for given tuple
int getCount(const unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, const tuple<int, int> &key)
{
    if (hash_table.count(key))
    {

        return hash_table.at(key);
    }

    else
    {
        return 0;
    }
}

int getCountX(const unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, int x)
{
    int count = 0;
    for (const auto &kv : hash_table)
    {
        if (get<0>(kv.first) == x)
        {
            count += kv.second;
        }
    }
    return count;
}

int getCountY(const unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, int y)
{
    int count = 0;
    for (auto it = hash_table.begin(); it != hash_table.end(); ++it)
    {
        const auto &key = it->first;
        const auto &value = it->second;
        if (get<1>(key) == y)
        {
            count += value;
        }
    }
    return count;
}

// add new tuple or increase count of existing tuple
void addTuple(unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, const tuple<int, int> &key)
{

    if (hash_table.count(key))
    {
        hash_table[key]++;
    }

    else
    {
        hash_table[key] = 1;
    }
}

// decrease count or remove tuple
void removeTuple(unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, const tuple<int, int> &key)
{

    if (hash_table.count(key))
    {

        if (hash_table[key] == 1)
        {
            hash_table.erase(key);
        }

        else
        {
            hash_table[key]--;
        }
    }
    else
    {
        cout << "Key (" << get<0>(key) << ", " << get<1>(key) << ") does not exist in the hash table" << endl;
    }
}
