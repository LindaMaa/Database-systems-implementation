#pragma once
using namespace std;

struct hash_tuple
{
    template <class T1, class T2>
    size_t operator()(const tuple<T1, T2> &tup) const;
};

namespace std
{
    template <typename T>
    struct hash<std::tuple<T, T>>
    {
        size_t operator()(const std::tuple<T, T> &t) const;
    };
}

int getCount(const unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, const tuple<int, int> &key);
int getCountX(const unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, int x);
int getCountY(const unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, int y);
void addTuple(unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, const tuple<int, int> &key);
void removeTuple(unordered_map<tuple<int, int>, int, hash_tuple> &hash_table, const tuple<int, int> &key);
