#pragma once
#include <exception>
#include <functional>
#include <iterator>
#include <list>
#include <vector>
#include <iostream>

namespace MyHashMap {
    template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
    class HashMap {
     private:
        typedef std::list<std::pair<KeyType, ValueType>> Cell;
        template <class OuterIter, class InnerIter>
        class base_iterator_ {
        public:
            base_iterator_() {}

            base_iterator_(OuterIter from_outer,
                           InnerIter from_inner,
                           OuterIter from_table_end)
                    : outer_(from_outer), inner_(from_inner), end_of_table_(from_table_end) {
                if (outer_ != end_of_table_ && inner_ == outer_->end())
                    ++(*this);
            }

            base_iterator_ &operator++() {
                if (outer_ == end_of_table_) {
                    return *this;
                }
                if (inner_ != outer_->end())
                    inner_++;
                while (outer_ != end_of_table_ && inner_ == outer_->end()) {
                    outer_++;
                    if (outer_ != end_of_table_)
                        inner_ = outer_->begin();
                }
                return *this;
            }

            base_iterator_ operator++(int) {
                base_iterator_ buff(*this);
                ++(*this);
                return buff;
            }

            bool operator==(const base_iterator_ &other) {
                if (outer_ == other.outer_ && outer_ == end_of_table_) return true;
                return outer_ == other.outer_ && inner_ == other.inner_;
            }

            bool operator!=(const base_iterator_ &other) {
                return !(*this == other);
            }

        protected:
            OuterIter outer_;
            InnerIter inner_;
            OuterIter end_of_table_;
        };

     public:
        explicit HashMap(Hash h = Hash()) : hasher(h) {}

        template<class InputIterator>
        HashMap(const InputIterator &begin, const InputIterator &end, Hash h = Hash()): HashMap(h) {
            for (auto it = begin; it != end; ++it)
                insert(*it);
        }

        HashMap(const std::initializer_list<std::pair<KeyType, ValueType>>& init, Hash h = Hash()) : HashMap(init.begin(),
                                                                                                      init.end(), h) {}

        size_t size() const {
            return len;
        }

        bool empty() const {
            return len == 0;
        }

        Hash hash_function() const {
            return hasher;
        }

        typedef typename std::vector<Cell>::iterator OuterNotConstIter;
        typedef typename Cell::iterator InnerNotConstIter;
        class iterator : public base_iterator_<OuterNotConstIter, InnerNotConstIter> {
            using base_iterator_<OuterNotConstIter, InnerNotConstIter>::outer_;
            using base_iterator_<OuterNotConstIter, InnerNotConstIter>::inner_;
            using base_iterator_<OuterNotConstIter, InnerNotConstIter>::end_of_table_;
            using base_iterator_<OuterNotConstIter, InnerNotConstIter>::base_iterator_;
        public:

            std::pair<const KeyType, ValueType> &operator*() {
                return reinterpret_cast<std::pair<const KeyType, ValueType> &>(*inner_);
            }

            std::pair<const KeyType, ValueType> *operator->() {
                return reinterpret_cast<std::pair<const KeyType, ValueType>*>(&*inner_);
            }
        };

        iterator begin() {
            if (!table.empty())
                return { table.begin(), table.front().begin(), table.end() };
            else
                return { table.end(), typename Cell::iterator(), table.end() };
        }

        iterator end() {
            return { table.end(), typename Cell::iterator(), table.end() };
        }

        typedef typename std::vector<Cell>::const_iterator OuterConstIter;
        typedef typename Cell::const_iterator InnerConstIter;
        class const_iterator : public base_iterator_<OuterConstIter, InnerConstIter> {
            using base_iterator_<OuterConstIter, InnerConstIter>::outer_;
            using base_iterator_<OuterConstIter, InnerConstIter>::inner_;
            using base_iterator_<OuterConstIter, InnerConstIter>::end_of_table_;
            using base_iterator_<OuterConstIter, InnerConstIter>::base_iterator_;
        public:
            const std::pair<const KeyType, ValueType> &operator*() const {
                return reinterpret_cast<const std::pair<const KeyType, ValueType> &>(*inner_);
            }

            const std::pair<const KeyType, ValueType> *operator->() const {
                return reinterpret_cast<const std::pair<const KeyType, ValueType>*>(&*inner_);
            }
        };

        const_iterator begin() const {
            if (!table.empty())
                return { table.cbegin(), table.front().cbegin(), table.cend() };
            else
                return { table.cend(), typename Cell::iterator(), table.cend() };
        }

        const_iterator end() const {
            return { table.end(), typename Cell::iterator(), table.end() };
        }

        void insert(const std::pair<KeyType, ValueType> &pr) {
            _check_len();
            _insert(pr);
        }

        void erase(const KeyType &key) {
            if (table.size() == 0)
                return;
            size_t k = hasher(key) % table.size();
            for (auto it = table[k].begin(); it != table[k].end(); ++it) {
                if (it->first == key) {
                    table[k].erase(it);
                    --len;
                    return;
                }
            }
        }

        iterator find(const KeyType &key) {
            if (table.size() == 0)
                return end();
            size_t k = hasher(key) % table.size();
            for (auto it = table[k].begin(); it != table[k].end(); ++it) {
                if (it->first == key) {
                    return {table.begin() + k, it, table.end()};
                }
            }
            return end();
        }

        const_iterator find(const KeyType &key) const {
            if (table.size() == 0)
                return end();
            size_t k = hasher(key) % table.size();
            for (auto it = table[k].begin(); it != table[k].end(); ++it) {
                if (it->first == key) {
                    return {table.begin() + k, it, table.end()};
                }
            }
            return end();
        }

        ValueType &operator[](const KeyType &key) {
            auto ind = find(key);
            if (ind == end()) {
                insert({key, ValueType()});
                ind = find(key);
            }
            return ind->second;
        }

        const ValueType &at(const KeyType &key) const {
            auto ind = find(key);
            if (ind == end())
                throw std::out_of_range("method at don't found key");
            return ind->second;
        }

        void clear() {
            table.clear();
            len = 0;
        }

    private:
        double max_load = 1.0 / 2;
        Hash hasher;
        size_t len = 0;
        std::vector <Cell> table;

        inline void _insert(const std::pair<KeyType, ValueType> &pr) {
            if (table.size() == 0) {
                table.resize(1);
            }
            size_t k = hasher(pr.first) % table.size();
            auto it = find(pr.first);
            if (it == end()) {
                table[k].push_front(pr);
                ++len;
            }
        }

        inline bool _check_len() {
            if (len < max_load * table.size())
                return false;
            size_t k = table.size();
            std::vector<Cell> buff = table;
            table.clear();
            table.resize(k << 1);
            len = 0;
            for (const auto& row : buff)
                for (const auto& elem : row)
                    _insert(elem);
            return true;
        }
    };
}
template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
        using HashMap = MyHashMap::HashMap<KeyType, ValueType, Hash>;

