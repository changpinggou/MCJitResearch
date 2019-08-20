//
//  objc-utils.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/9.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef objc_utils_h
#define objc_utils_h

#include <functional>

namespace zq { namespace objc {
    
    //------------------------------entsize64_list_tt---------------------------------
    template <typename Element, typename List, uint32_t FlagMask>
    struct entsize64_list_tt {
        uint32_t entsizeAndFlags;
        uint32_t count;
        Element first;
        
        uint32_t entsize() const {
            return entsizeAndFlags & ~FlagMask;
        }
        uint32_t flags() const {
            return entsizeAndFlags & FlagMask;
        }
        
        Element& getOrEnd(uint32_t i) const {
            assert(i <= count);
            return *(Element *)((uint8_t *)&first + i*entsize());
        }
        Element& get(uint32_t i) const {
            assert(i < count);
            return getOrEnd(i);
        }
        
        size_t byteSize() const {
            return byteSize(entsize(), count);
        }
        
        static size_t byteSize(uint32_t entsize, uint32_t count) {
            return sizeof(entsize64_list_tt) + (count-1)*entsize;
        }
        
        List *duplicate() const {
            auto *dup = (List *)calloc(this->byteSize(), 1);
            dup->entsizeAndFlags = this->entsizeAndFlags;
            dup->count = this->count;
            std::copy(begin(), end(), dup->begin());
            return dup;
        }
        
        struct iterator;
        const iterator begin() const {
            return iterator(*static_cast<const List*>(this), 0);
        }
        iterator begin() {
            return iterator(*static_cast<const List*>(this), 0);
        }
        const iterator end() const {
            return iterator(*static_cast<const List*>(this), count);
        }
        iterator end() {
            return iterator(*static_cast<const List*>(this), count);
        }
        
        struct iterator {
            uint32_t entsize;
            uint32_t index;  // keeping track of this saves a divide in operator-
            Element* element;
            
            typedef std::random_access_iterator_tag iterator_category;
            typedef Element value_type;
            typedef ptrdiff_t difference_type;
            typedef Element* pointer;
            typedef Element& reference;
            
            iterator() { }
            
            iterator(const List& list, uint32_t start = 0)
            : entsize(list.entsize())
            , index(start)
            , element(&list.getOrEnd(start))
            { }
            
            const iterator& operator += (ptrdiff_t delta) {
                element = (Element*)((uint8_t *)element + delta*entsize);
                index += (int32_t)delta;
                return *this;
            }
            const iterator& operator -= (ptrdiff_t delta) {
                element = (Element*)((uint8_t *)element - delta*entsize);
                index -= (int32_t)delta;
                return *this;
            }
            const iterator operator + (ptrdiff_t delta) const {
                return iterator(*this) += delta;
            }
            const iterator operator - (ptrdiff_t delta) const {
                return iterator(*this) -= delta;
            }
            
            iterator& operator ++ () { *this += 1; return *this; }
            iterator& operator -- () { *this -= 1; return *this; }
            iterator operator ++ (int) {
                iterator result(*this); *this += 1; return result;
            }
            iterator operator -- (int) {
                iterator result(*this); *this -= 1; return result;
            }
            
            ptrdiff_t operator - (const iterator& rhs) const {
                return (ptrdiff_t)this->index - (ptrdiff_t)rhs.index;
            }
            
            Element& operator * () const { return *element; }
            Element* operator -> () const { return element; }
            
            operator Element& () const { return *element; }
            
            bool operator == (const iterator& rhs) const {
                return this->element == rhs.element;
            }
            bool operator != (const iterator& rhs) const {
                return this->element != rhs.element;
            }
            
            bool operator < (const iterator& rhs) const {
                return this->element < rhs.element;
            }
            bool operator > (const iterator& rhs) const {
                return this->element > rhs.element;
            }
        };
    };
    //------------------------------entsize64_list_tt---------------------------------
    
    
    //------------------------------list64_array_tt---------------------------------
    template <typename Element, typename List>
    class list64_array_tt {
        struct array_t {
            uint32_t count;
            List* lists[0];
            
            static size_t byteSize(uint32_t count) {
                return sizeof(array_t) + count*sizeof(lists[0]);
            }
            size_t byteSize() {
                return byteSize(count);
            }
        };
        
    protected:
        class iterator {
            List **lists;
            List **listsEnd;
            typename List::iterator m, mEnd;
            
        public:
            iterator(List **begin, List **end)
            : lists(begin), listsEnd(end)
            {
                if (begin != end) {
                    m = (*begin)->begin();
                    mEnd = (*begin)->end();
                }
            }
            
            const Element& operator * () const {
                return *m;
            }
            Element& operator * () {
                return *m;
            }
            
            bool operator != (const iterator& rhs) const {
                if (lists != rhs.lists) return true;
                if (lists == listsEnd) return false;  // m is undefined
                if (m != rhs.m) return true;
                return false;
            }
            
            const iterator& operator ++ () {
                assert(m != mEnd);
                m++;
                if (m == mEnd) {
                    assert(lists != listsEnd);
                    lists++;
                    if (lists != listsEnd) {
                        m = (*lists)->begin();
                        mEnd = (*lists)->end();
                    }
                }
                return *this;
            }
        };
        
    private:
        union {
            List* list;
            uintptr_t arrayAndFlag;
        };
        
        bool hasArray() const {
            return arrayAndFlag & 1;
        }
        
        array_t *array() {
            return (array_t *)(arrayAndFlag & ~1);
        }
        
        void setArray(array_t *array) {
            arrayAndFlag = (uintptr_t)array | 1;
        }
        
    public:
        
        uint32_t count() {
            uint32_t result = 0;
            for (auto lists = beginLists(), end = endLists();
                 lists != end;
                 ++lists)
            {
                result += (*lists)->count;
            }
            return result;
        }
        
        iterator begin() {
            return iterator(beginLists(), endLists());
        }
        
        iterator end() {
            List **e = endLists();
            return iterator(e, e);
        }
        
        
        uint32_t countLists() {
            if (hasArray()) {
                return array()->count;
            } else if (list) {
                return 1;
            } else {
                return 0;
            }
        }
        
        List** beginLists() {
            if (hasArray()) {
                return array()->lists;
            } else {
                return &list;
            }
        }
        
        List** endLists() {
            if (hasArray()) {
                return array()->lists + array()->count;
            } else if (list) {
                return &list + 1;
            } else {
                return &list;
            }
        }
        
        void attachLists(List* const * addedLists, uint32_t addedCount) {
            if (addedCount == 0) return;
            
            if (hasArray()) {
                // many lists -> many lists
                uint32_t oldCount = array()->count;
                uint32_t newCount = oldCount + addedCount;
                setArray((array_t *)realloc(array(), array_t::byteSize(newCount)));
                array()->count = newCount;
                memmove(array()->lists + addedCount, array()->lists,
                        oldCount * sizeof(array()->lists[0]));
                memcpy(array()->lists, addedLists,
                       addedCount * sizeof(array()->lists[0]));
            }
            else if (!list  &&  addedCount == 1) {
                // 0 lists -> 1 list
                list = addedLists[0];
            }
            else {
                // 1 list -> many lists
                List* oldList = list;
                uint32_t oldCount = oldList ? 1 : 0;
                uint32_t newCount = oldCount + addedCount;
                setArray((array_t *)malloc(array_t::byteSize(newCount)));
                array()->count = newCount;
                if (oldList) array()->lists[addedCount] = oldList;
                memcpy(array()->lists, addedLists,
                       addedCount * sizeof(array()->lists[0]));
            }
        }
        
        void tryFree() {
            if (hasArray()) {
                for (uint32_t i = 0; i < array()->count; i++) {
                    try_free(array()->lists[i]);
                }
                try_free(array());
            }
            else if (list) {
                try_free(list);
            }
        }
        
        template<typename Result>
        Result duplicate() {
            Result result;
            
            if (hasArray()) {
                array_t *a = array();
                result.setArray((array_t *)memdup(a, a->byteSize()));
                for (uint32_t i = 0; i < a->count; i++) {
                    result.array()->lists[i] = a->lists[i]->duplicate();
                }
            } else if (list) {
                result.list = list->duplicate();
            } else {
                result.list = nil;
            }
            
            return result;
        }
    };
    //------------------------------list64_array_tt---------------------------------
}}


#endif /* objc_utils_h */
