#include <iostream>
#include <cmath>
#include <algorithm>
#include <initializer_list>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std;

template <class KeyType, class ValueType, class Hash = hash<KeyType>> class HashMap {
private:
    list<pair<const KeyType, ValueType>>* list_iter = new list<pair<const KeyType, ValueType>>();
    const typename list<pair<const KeyType, ValueType>>::iterator End = list_iter->begin();
    const typename list<pair<const KeyType, ValueType>>::const_iterator End_const = list_iter->begin();
    list<list<pair<const KeyType, ValueType>>*>* cur_sized = new list<list<pair<const KeyType, ValueType>>*>();
    const typename list<list<pair<const KeyType, ValueType>>*>::iterator e_table = cur_sized->begin();
    Hash hasher;
    size_t cur_size = 0;
    vector<typename list<list<pair<const KeyType, ValueType>>*>::iterator> table;
    list<list<pair<const KeyType, ValueType>>*> links;
    void _insert(const pair<KeyType, ValueType>& pr) {
        if (table.size() == 0) {
            table.resize(1, e_table);
        }
        size_t k = hasher(pr.first) % table.size();
        if (table[k] == e_table) {
            links.push_back(new list<pair<const KeyType, ValueType>>());
            auto it = links.end();
            table[k] = --it;
        }
        if (find(pr.first) == end()) {
            (*table[k])->push_back(pr);
            ++cur_size;
        }
    }
    bool _check_cur_size() {
        if (2 * cur_size < table.size()) {
            return false;
        }
        size_t k = table.size();
        table.clear();
        table.resize(k << 1, e_table);
        list<list<pair<const KeyType, ValueType>>*> buff = move(links);
        links.clear();
        cur_size = 0;
        for (typename list<list<pair<const KeyType, ValueType>>*>::iterator ind = buff.begin(); ind != buff.end(); ++ind) {
            for (const auto& elem : **ind) {
                _insert(elem);
            }
            delete *ind;
        }
        return true;
    }
public:
    HashMap(Hash h = Hash()): hasher(h) {}
    size_t size() const {
        return cur_size;
    }
    bool empty() const {
        return cur_size == 0;
    }
    Hash hash_function() const {
        return hasher;
    }
    template <class Iter>
    HashMap(const Iter& begin, const Iter& end, Hash h = Hash()): hasher(h) {
        auto x = begin;
        size_t cnt = 0;
        while (x != end) {
            cnt++;
            x++;
        }
        table.resize(cnt << 1, e_table);
        for (Iter it = begin; it != end; ++it)
            _insert(*it);
    }
    HashMap(initializer_list<pair<KeyType, ValueType>> init, Hash h = Hash()): hasher(h) {
        table.resize(init.size() << 1, e_table);
        for (const auto& it : init) {
            _insert(it);
        }
    }
    HashMap(const HashMap& mp) {
        table.resize(mp.size() << 1, e_table);
        for (const auto& it : mp)
            _insert(it);
    }
    HashMap& operator=(const HashMap& other) {
        if (&other == this) {
            return *this;
        }
        table.clear();
        table.resize(other.size() << 1, e_table);
        for (list<pair<const KeyType, ValueType>>* link : links) {
            delete link;
        }
        cur_size = 0;
        links.clear();
        for (auto it : other)
            _insert(it);
        return *this;
    }
    ~HashMap() {
        for (list<pair<const KeyType, ValueType>>* link : links) {
            delete link;
        }
        delete list_iter;
        delete cur_sized;
        links.clear();
    }

    class iterator : public std::iterator<input_iterator_tag, ValueType> {
        list<list<pair<const KeyType, ValueType>>*>* links;
        typename list<list<pair<const KeyType, ValueType>>*>::iterator link;
        typename list<pair<const KeyType, ValueType>>::iterator elem;
        friend void HashMap::erase(const KeyType& key);
    public:
        iterator() : links(nullptr) {}
        iterator(list<list<pair<const KeyType, ValueType>>*>* links, typename list<list<pair<const KeyType, ValueType>>*>::iterator link, typename list<pair<const KeyType, ValueType>>::iterator elem) : links(links), link(link), elem(elem) {}
        pair<const KeyType, ValueType>& operator*() {
            return *elem;
        }
        typename list<pair<const KeyType, ValueType>>::iterator operator->() {
            return elem;
        }
        iterator& operator++() {
            ++elem;
            if (elem == (*link)->end()) {
                ++link;
                if (link != links->end())
                    elem = (*link)->begin();
            }
            return *this;
        }
        iterator operator++(int) {
            iterator buff = *this;
            ++elem;
            if (elem == (*link)->end()) {
                ++link;
                if (link != links->end())
                    elem = (*link)->begin();
            }
            return buff;
        }
        bool operator!=(const iterator& other) {
            return links != other.links || link != other.link ||
                   (link != links->end() && *elem != *(other.elem));
        }
        bool operator==(const iterator& other) {
            return links == other.links && link == other.link &&
                   (link == links->end() || *elem == *(other.elem));
        }
    };
    iterator begin() {
        return {&links, links.begin(), links.empty() ? End : (*links.begin())->begin()};
    }
    iterator end() {
        return {&links, links.end(), End};
    }

    class const_iterator : public std::iterator<input_iterator_tag, ValueType> {
        const list<list<pair<const KeyType, ValueType>>*>* links;
        typename list<list<pair<const KeyType, ValueType>>*>::const_iterator link;
        typename list<pair<const KeyType, ValueType>>::const_iterator elem;
    public:
        const_iterator() : links(nullptr) {}
        const_iterator(const list<list<pair<const KeyType, ValueType>>*>* links, typename list<list<pair<const KeyType, ValueType>>*>::const_iterator link, typename list<pair<const KeyType, ValueType>>::const_iterator elem) : links(links), link(link), elem(elem) {}
        const pair<const KeyType, ValueType>& operator*() const {
            return *elem;
        }
        typename list<pair<const KeyType, ValueType>>::const_iterator operator->() {
            return elem;
        }
        const_iterator& operator++() {
            ++elem;
            if (elem == (*link)->end()) {
                ++link;
                if (link != links->end())
                    elem = (*link)->begin();
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator buff = *this;
            ++elem;
            if (elem == (*link)->end()) {
                ++link;
                if (link != links->end())
                    elem = (*link)->begin();
            }
            return buff;
        }
        bool operator!=(const const_iterator& other) {
            return links != other.links || link != other.link ||
                   (link != links->end() && elem != other.elem);
        }
        bool operator==(const const_iterator& other) {
            return links == other.links && link == other.link &&
                   (link == links->end() || elem == other.elem);
        }
    };
    const_iterator begin() const {
        return {&links, links.begin(), links.empty() ? End_const : (*(links.begin()))->begin()};
    }
    const_iterator end() const {
        return {&links, links.end(), End_const};
    }

    void insert(const pair<KeyType, ValueType>& pr) {
        _insert(pr);
        _check_cur_size();
    }
    void erase(const KeyType& key) {
        iterator ind = find(key);
        if (ind == end())
            return;
        (*ind.link)->erase(ind.elem);
        --cur_size;
        if ((*ind.link)->empty()) {
            size_t k = hasher(key) % table.size();
            table[k] = e_table;
            delete *ind.link;
            links.erase(ind.link);
        }
    }
    iterator find(const KeyType& key) {
        if (table.size() == 0)
            return end();
        size_t k = hasher(key) % table.size();
        if (table[k] == e_table) {
            return end();
        }
        for (typename list<pair<const KeyType, ValueType>>::iterator ind = (*table[k])->begin(); ind != (*table[k])->end(); ++ind)
            if (ind->first == key)
                return {&links, table[k], ind};
        return end();
    }
    const_iterator find(const KeyType& key) const {
        if (table.size() == 0)
            return end();
        size_t k = hasher(key) % table.size();
        if (table[k] == e_table) {
            return end();
        }
        for (typename list<pair<const KeyType, ValueType>>::const_iterator ind = (*table[k])->begin(); ind != (*table[k])->end(); ++ind)
            if (ind->first == key)
                return {&links, table[k], ind};
        return end();
    }
    ValueType& operator[](const KeyType& key) {
        auto ind = find(key);
        if (ind == end()) {
            insert({key, ValueType()});
            ind = find(key);
        }
        return ind->second;
    }
    const ValueType& at(const KeyType& key) const {
        auto ind = find(key);
        if (ind == end())
            throw out_of_range("error");
        return ind->second;
    }
    void clear() {
        table.clear();
        for (const auto& el : links) {
            delete el;
        }
        links.clear();
        cur_size = 0;
    }
};
