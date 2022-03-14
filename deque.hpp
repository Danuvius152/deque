#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include <cstddef>

#include "exceptions.hpp"

namespace sjtu {

template <class T>
class deque {
   private:
    int BLOCK_SIZE = 160;
    int THRESHOLD = 128;
    class Element {
       public:
        T *value;
        Element *preele = nullptr;
        Element *nextele = nullptr;

        explicit Element(const T &_value, Element *_pre = nullptr,
                         Element *_next = nullptr)
            : preele(_pre), nextele(_next) {
            value = new T(_value);
        }
        Element(const Element &other) { value = new T(*(other.value)); }

        ~Element() {
            if (value != nullptr) delete value;
            Element *preele = nullptr;
            Element *nextele = nullptr;
        }
    };
    class Block {
       public:
        int elementNum = 0;
        Block *preB = nullptr;
        Block *nextB = nullptr;
        Element *eleHead = nullptr;
        Element *eleTail = nullptr;

        Block(int _elementNum, Block *_pre, Block *_next, Element *_head,
              Element *_tail)
            : elementNum(_elementNum),
              preB(_pre),
              nextB(_next),
              eleHead(_head),
              eleTail(_tail) {}
        Block(const Block &other) {
            Element *cur = other.eleHead;
            while (cur != nullptr) {
                if (eleTail != nullptr) {
                    eleTail->nextele = new Element(*cur);
                    eleTail->nextele->preele = eleTail;
                    eleTail = eleTail->nextele;
                } else {
                    eleHead = new Element(*cur);
                    eleTail = eleHead;
                }
                elementNum++;
                cur = cur->nextele;
            }
        }

        Block *splitBlock(int x) {
            Block *newBlock =
                new Block(elementNum - x, this, nextB, nullptr, nullptr);
            elementNum = x;
            if (nextB != nullptr) nextB->preB = newBlock;
            nextB = newBlock;
            Element *tmp = findElement(x);
            newBlock->eleHead = tmp;
            newBlock->eleTail = eleTail;
            eleTail = tmp->preele;
            newBlock->eleHead->preele = nullptr;
            eleTail->nextele = nullptr;
            return newBlock;  // x为newBlock的头
        }

        void mergeBlock() {
            // if (nextB == nullptr) return;
            if (elementNum == 0) {
                eleHead = nextB->eleHead;
                eleTail = nextB->eleTail;
            } else if (nextB->elementNum == 0) {
            } else {
                eleTail->nextele = nextB->eleHead;
                nextB->eleHead->preele = eleTail;
                eleTail = nextB->eleTail;
            }
            elementNum += nextB->elementNum;
            nextB->eleHead = nullptr;
            nextB->eleTail = nullptr;
            if (nextB->nextB != nullptr) nextB->nextB->preB = this;
            Block *tmp = nextB;
            nextB = nextB->nextB;
            delete tmp;
        }
        Element *findElement(int index) {
            Element *temp = eleHead;
            for (int i = 0; i < index; i++) temp = temp->nextele;
            return temp;
        }
        void insertElement(deque<T> *dq, int index, Element *t,
                           int SIZE) {  //插完在index处
            if (elementNum == 0) {
                eleHead = t;
                eleTail = t;
            } else {
                if (index == 0) {
                    t->preele = nullptr;
                    t->nextele = eleHead;
                    eleHead->preele = t;
                    eleHead = t;
                } else if (index == elementNum) {
                    eleTail->nextele = t;
                    t->preele = eleTail;
                    t->nextele = nullptr;
                    eleTail = t;
                } else {
                    Element *tmp = findElement(index);
                    tmp->preele->nextele = t;
                    t->preele = tmp->preele;
                    t->nextele = tmp;
                    tmp->preele = t;
                }
            }
            elementNum++;
            dq->len++;
            if (elementNum == SIZE) {
                Block *newblock = splitBlock(SIZE >> 1);
                if (this == dq->blockTail) dq->blockTail = newblock;
                dq->blockNum++;
            }
        };
        void eraseElement(deque<T> *dq, int index, int merge_size) {
            if (elementNum == 0) throw container_is_empty();
            if (elementNum == 1) {
                delete eleHead;
                eleHead = nullptr;
                eleTail = nullptr;
            } else {
                if (index == 0) {
                    Element *tmp = eleHead;
                    eleHead = eleHead->nextele;
                    eleHead->preele = nullptr;
                    delete tmp;
                } else if (index >= elementNum - 1) {
                    Element *tmp = eleTail;
                    eleTail = eleTail->preele;
                    eleTail->nextele = nullptr;
                    delete tmp;
                } else {
                    Element *tmp = findElement(index);
                    tmp->preele->nextele = tmp->nextele;
                    tmp->nextele->preele = tmp->preele;
                    delete tmp;
                }
            }
            elementNum--;
            dq->len--;
            if (elementNum == 0) {
                if (nextB != nullptr) {
                    if (this == dq->blockTail->preB) dq->blockTail = this;
                    mergeBlock();
                    dq->blockNum--;
                }
                if (preB != nullptr) {
                    if (this == dq->blockTail) dq->blockTail = this->preB;
                    preB->mergeBlock();
                    dq->blockNum--;
                }
            } else if (nextB != nullptr &&
                       elementNum + nextB->elementNum < merge_size) {
                if (this == dq->blockTail->preB) dq->blockTail = this;
                mergeBlock();
                dq->blockNum--;
            }
        }
        ~Block() {
            Element *tmp;
            while (eleHead != nullptr) {
                tmp = eleHead;
                eleHead = eleHead->nextele;
                delete tmp;
            }
        }
    };
    Block *blockHead = nullptr;
    Block *blockTail = nullptr;
    int len = 0;
    int blockNum = 0;
    void initialize() {
        blockHead = new Block(0, nullptr, nullptr, nullptr, nullptr);
        blockTail = blockHead;
        blockNum = 1, len = 0;
        BLOCK_SIZE = 160, THRESHOLD = 128;
    }
    Block *findBlock(const int pos,
                     int &index) const {  //返回的index是当前Block的
        if (pos == 0) {
            index = 0;
            return blockHead;
        } else if (pos == len - 1) {
            index = blockTail->elementNum - 1;
            return blockTail;
        }
        Block *cur = blockHead;
        int cnt = pos;
        while (cur->elementNum <= cnt) {
            cnt -= cur->elementNum;
            if (cur->nextB == nullptr) break;
            cur = cur->nextB;
        }
        index = cnt;
        return cur;
    }
    void pushbackBlock(Block *t) {
        if (blockTail != nullptr) {
            blockTail->nextB = new Block(*t);
            blockTail->nextB->preB = blockTail;
            blockTail = blockTail->nextB;
        } else {
            blockHead = new Block(*t);
            blockTail = blockHead;
        }
        blockNum++;
        len += t->elementNum;
    }
    void adjustSize() {
        if (blockNum == BLOCK_SIZE * 4) {
            BLOCK_SIZE *= 2, THRESHOLD *= 2;
            blockTail = blockTail->preB;  //偶数
            Block *cur = blockHead;
            while (cur != nullptr) {
                cur->mergeBlock();
                cur = cur->nextB;
            }
        } else if (BLOCK_SIZE > 160 && blockNum * 4 == BLOCK_SIZE) {
            BLOCK_SIZE /= 2, THRESHOLD /= 2;
            Block *cur = blockHead;
            while (cur != nullptr) {
                cur->splitBlock(cur->elementNum / 2);
                cur = cur->nextB, cur = cur->nextB;
            }
            blockTail = blockTail->nextB;
        }
    }

   public:
    class const_iterator;
    class iterator {
        friend class deque;

       private:
        deque<T> *ptr = nullptr;
        Block *curBlock = nullptr;
        Element *curElement = nullptr;
        int total = -1;

       public:
        iterator() = default;
        iterator(deque<T> *_ptr, Block *block, Element *element, int _total)
            : ptr(_ptr), curBlock(block), curElement(element), total(_total) {}
        iterator(const iterator &other)
            : ptr(other.ptr),
              curBlock(other.curBlock),
              curElement(other.curElement),
              total(other.total) {}
        iterator(deque<T> *_ptr, int _total) : ptr(_ptr), total(_total) {
            if (total > ptr->len)
                throw invalid_iterator();
            else if (total == ptr->len) {
                curBlock = nullptr;
                curElement = nullptr;
            } else {
                if (ptr->blockNum == 0) ptr->initialize();
                int index;
                curBlock = ptr->findBlock(total, index);
                curElement = curBlock->findElement(index);
            }
        }
        iterator &operator=(const iterator &other) {
            if (this == &other) return *this;
            ptr = other.ptr;
            curBlock = other.curBlock;
            curElement = other.curElement;
            total = other.total;
            return *this;
        }
        iterator operator+(const int &n) const {
            return iterator(ptr, total + n);
        }
        iterator operator-(const int &n) const {
            return iterator(ptr, total - n);
        }
        int operator-(const iterator &rhs) const {
            if (ptr != rhs.ptr) throw invalid_iterator();
            return total - rhs.total;
        }
        iterator &operator+=(const int &n) {
            *this = *this + n;
            return *this;
        }
        iterator &operator-=(const int &n) {
            *this = *this - n;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            (*this) += 1;
            return tmp;
        }
        iterator &operator++() {
            (*this) += 1;
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            *this -= 1;
            return tmp;
        }
        iterator &operator--() {
            *this -= 1;
            return *this;
        }

        T &operator*() const {
            if (total >= ptr->len || total < 0) throw invalid_iterator();
            return *(curElement->value);
        }
        T *operator->() const noexcept {
            if (total >= ptr->len || total < 0) throw invalid_iterator();
            return curElement->value;
        }
        bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr && curBlock == rhs.curBlock &&
                   curElement == rhs.curElement && total == rhs.total;
        }
        bool operator==(const const_iterator &rhs) const {
            return ptr == rhs.ptr && curBlock == rhs.curBlock &&
                   curElement == rhs.curElement && total == rhs.total;
        }
        bool operator!=(const iterator &rhs) const { return !((*this) == rhs); }
        bool operator!=(const const_iterator &rhs) const {
            return !((*this) == rhs);
        }
    };
    class const_iterator {
        friend class deque;

       private:
        const deque<T> *ptr = nullptr;
        Block *curBlock = nullptr;
        Element *curElement = nullptr;
        int total = -1;

       public:
        const_iterator() = default;
        const_iterator(const deque<T> *_ptr, Block *block, Element *element,
                       int _total)
            : ptr(_ptr), curBlock(block), curElement(element), total(_total) {}
        const_iterator(const const_iterator &other)
            : ptr(other.ptr),
              curBlock(other.curBlock),
              curElement(other.curElement),
              total(other.total) {}
        const_iterator(const deque<T> *_ptr, int _total)
            : ptr(_ptr), total(_total) {
            if (total > ptr->len)
                throw invalid_iterator();
            else if (total == ptr->len) {
            } else {
                if (ptr->blockNum == 0) throw invalid_iterator();
                int index;
                curBlock = ptr->findBlock(total, index);
                curElement = curBlock->findElement(index);
            }
        }
        const_iterator &operator=(const const_iterator &other) {
            if (this == &other) return *this;
            ptr = other.ptr;
            curBlock = other.curBlock;
            curElement = other.curElement;
            total = other.total;
            return *this;
        }
        const_iterator operator+(const int &n) const {
            return const_iterator(ptr, total + n);
        }
        const_iterator operator-(const int &n) const {
            return const_iterator(ptr, total - n);
        }
        int operator-(const const_iterator &rhs) const {
            if (ptr != rhs.ptr) throw invalid_iterator();
            return total - rhs.total;
        }
        const_iterator &operator+=(const int &n) {
            *this = *this + n;
            return *this;
        }
        const_iterator &operator-=(const int &n) {
            *this = *this - n;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            *this += 1;
            return tmp;
        }
        const_iterator &operator++() {
            *this += 1;
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            *this -= 1;
            return tmp;
        }
        const_iterator &operator--() {
            *this -= 1;
            return *this;
        }

        T &operator*() const {
            if (total >= ptr->len || total < 0) throw invalid_iterator();
            return *(curElement->value);
        }
        T *operator->() const noexcept {
            if (total >= ptr->len || total < 0) throw invalid_iterator();
            return curElement->value;
        }
        bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr && curBlock == rhs.curBlock &&
                   curElement == rhs.curElement && total == rhs.total;
        }
        bool operator==(const const_iterator &rhs) const {
            return ptr == rhs.ptr && curBlock == rhs.curBlock &&
                   curElement == rhs.curElement && total == rhs.total;
        }
        bool operator!=(const iterator &rhs) const { return !((*this) == rhs); }
        bool operator!=(const const_iterator &rhs) const {
            return !((*this) == rhs);
        }
    };

    deque() { initialize(); }
    deque(const deque &other) {
        Block *cur = other.blockHead;
        while (cur != nullptr) {
            pushbackBlock(cur);
            cur = cur->nextB;
        }
    }
    ~deque() {
        Block *cur;
        while (blockHead != nullptr) {
            cur = blockHead;
            blockHead = blockHead->nextB;
            delete cur;
        }
    }
    deque &operator=(const deque &other) {
        if (this == &other) return *this;
        Block *cur;
        while (blockHead != nullptr) {
            cur = blockHead;
            blockHead = blockHead->nextB;
            delete cur;
        }
        blockTail = nullptr;
        len = 0;
        blockNum = 0;
        cur = other.blockHead;
        while (cur != nullptr) {
            pushbackBlock(cur);
            cur = cur->nextB;
        }
        return *this;
    }
    T &at(const size_t &pos) {
        if (pos < 0 || pos >= len) throw index_out_of_bound();
        int index;
        Block *x = findBlock(pos, index);
        Element *t = x->findElement(index);
        return *(t->value);
    }
    const T &at(const size_t &pos) const {
        if (pos < 0 || pos >= len) throw index_out_of_bound();
        int index;
        Block *x = findBlock(pos, index);
        Element *t = x->findElement(index);
        return *(t->value);
    }
    T &operator[](const size_t &pos) { return at(pos); }
    const T &operator[](const size_t &pos) const { return at(pos); }

    const T &front() const {
        if (len == 0) throw container_is_empty();
        return *(blockHead->eleHead->value);
    }
    const T &back() const {
        if (len == 0) throw container_is_empty();
        return *(blockTail->eleTail->value);
    }
    iterator begin() { return iterator(this, 0); }
    const_iterator cbegin() const { return const_iterator(this, 0); }
    iterator end() { return iterator(this, len); }
    const_iterator cend() const { return const_iterator(this, len); }

    bool empty() const { return len == 0; }
    size_t size() const { return len; }

    void clear() {
        Block *cur;
        while (blockHead != nullptr) {
            cur = blockHead;
            blockHead = blockHead->nextB;
            delete cur;
        }
        initialize();
    }
    /**
     * inserts elements at the specified locat on in the container.
     * inserts value before pos
     * returns an iterator pointing to the inserted value
     */
    iterator insert(iterator pos, const T &value) {
        adjustSize();
        if (blockNum == 0) initialize();
        int t = pos.total;
        if (t < 0 || t > len || pos.ptr != this) throw invalid_iterator();
        if (t == len) {
            push_back(value);
            return iterator(this, blockTail, blockTail->eleTail, len - 1);
        }
        int index;
        Block *cur = findBlock(t, index);
        Element *tmp = new Element(value);
        cur->insertElement(this, index, tmp, BLOCK_SIZE);
        return iterator(this, t);
    }
    /**
     * removes the element at pos.
     * returns an iterator pointing to the following element,
     * if pos pointing to the last element, end() will be returned
     * throw if the container is empty, the iterator is invalid or it
     * points to a wrong place.
     */
    iterator erase(iterator pos) {
        adjustSize();
        int t = pos.total;
        if (blockNum == 0) initialize();
        if (len == 0) throw container_is_empty();
        if (t < 0 || t >= len || pos.ptr != this) throw invalid_iterator();
        int index;
        Block *cur = findBlock(t, index);
        cur->eraseElement(this, index, THRESHOLD);
        if (t == len)
            return iterator(this, nullptr, nullptr, len);
        else
            return iterator(this, t);
    }
    void push_back(const T &value) {
        adjustSize();
        if (blockNum == 0) initialize();
        blockTail->insertElement(this, blockTail->elementNum,
                                 new Element(value), BLOCK_SIZE);
    }
    void pop_back() {
        adjustSize();
        if (blockNum == 0) initialize();
        if (len == 0) throw container_is_empty();
        blockTail->eraseElement(this, blockTail->elementNum, THRESHOLD);
    }
    void push_front(const T &value) {
        adjustSize();
        if (blockNum == 0) initialize();
        blockHead->insertElement(this, 0, new Element(value), BLOCK_SIZE);
    }
    void pop_front() {
        adjustSize();
        if (blockNum == 0) initialize();
        blockHead->eraseElement(this, 0, THRESHOLD);
    }
};

}  // namespace sjtu


#endif
