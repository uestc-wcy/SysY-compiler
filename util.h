#ifndef UTIL_H
#define UTIL_H
/*
#include <list>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <set>

std::vector<std::string> str_split(const std::string &s, const std::string &seperator);

template <class Tk, class Tp>
std::vector<Tk> get_keys_from_value(const std::map<Tk, Tp> &map, const Tp &e)
{
    std::vector<Tk> keys;
    for (auto p = map.begin(); p != map.end(); ++p)
    {
        if (p->second == e)
        {
            keys.push_back(p->first);
        }
    }
    return keys;
}

template <class Tk, class Tp>
std::vector<Tk> get_keys(const std::map<Tk, Tp> &map)
{
    std::vector<Tk> keys;
    for (auto p = map.begin(); p != map.end(); ++p)
        keys.push_back(p->first);

    return keys;
}

template <class T>
bool is_child(const std::set<T> &a, const std::set<T> &b) //b是否为a的子集
{
    for (T e : b)
        if (a.find(e) == a.end())
            return false;

    return true;
}

template <class T>
bool find(const std::vector<T> &vec, const T &a)
{
    for (auto e : vec)
        if (e == a)
            return true;

    return false;
}

template <class T>
void erase(std::vector<T> &vec, const T &a)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        if (vec[i] == a)
        {
            vec.erase(vec.begin() + i);
            break;
        }
    }
}

template <class T>
std::vector<T> to_array(const std::set<T> &set)
{
    std::vector<T> vec;
    for (auto e : set)
        vec.push_back(e);
    return vec;
}*/


#endif // UTIL_H