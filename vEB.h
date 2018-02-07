
#ifndef VEB_V2_VEB_H
#define VEB_V2_VEB_H

#define NIL UINT32_MAX
#define uint unsigned int

using namespace std;

template <uint k>
class vEB {
protected:
    vEB<(k >> 1)> *summary;
    vEB<(k >> 1)> *cluster[1UL << (k >> 1)];
    uint min, max;
public:

    vEB() : min(NIL), max(NIL), summary(nullptr) {}

    __attribute__((transaction_safe))
    inline uint getMin() const {
        return min;
    }

    __attribute__((transaction_safe))
    inline uint getMax() const {
        return max;
    }

    __attribute__((transaction_safe))
    inline bool empty() {
        return min == NIL;
    }

    __attribute__((transaction_safe))
    bool _lookup(uint key) {
        if (key == min || key == max) {
            return true;
        } else if (k > 1) {
            uint hi = high(key);
            uint lo = low(key);
            if (cluster[hi] != nullptr)
                return cluster[hi]->_lookup(lo);
        }
        return false;
    }

    __attribute__((transaction_safe))
    void _insert(uint key) {
        if (empty()) {
            min = key;
            max = key;
        } else {
            if (key < min) {
                uint temp = min;
                min = key;
                key = temp;
            }
            if (key > max)
                max = key;
            if (k > 1) {
                uint hi = high(key);
                uint lo = low(key);

                if (cluster[hi] == nullptr) {
                    cluster[hi] = new vEB<(k >> 1)>();
                }
                if (cluster[hi]->empty()) {
                    if (summary == nullptr)
                        summary = new vEB<(k >> 1)>();
                    summary->_insert(hi);
                }
                cluster[hi]->_insert(lo);
            }
        }

    }


    __attribute__((transaction_safe))
    void _remove(uint key) {
        if (min == max) {
            min = max = NIL;
        } else if (k == 1) {
            if (key == 0)
                max = min = 1;
            else
                max = min = 0;
        } else {
            if (key == min) {
                uint first_cluster = summary->getMin();
                key = merge(first_cluster, cluster[first_cluster]->getMin());
                min = key;
            }

            uint hi = high(key);
            uint lo = low(key);
            cluster[hi]->_remove(lo);

            if (cluster[hi]->getMin() == NIL) {
                summary->_remove(hi);
                if (key == max) {
                    uint summary_max = summary->getMax();
                    if (summary_max == NIL)
                        max = min;
                    else
                        max = merge(summary_max, cluster[summary_max]->getMax());
                }
            } else if (key == max) {
                max = merge(hi, cluster[hi]->getMax());
            }
        }
    }

    __attribute__((transaction_safe))
    uint successor(uint key) {
        if (k == 1) {
            if (key == 0 && max == 1)
                return 1;
            return NIL;
        }
        if (empty() || key > max) {
            return NIL;
        }
        if (key < min && min != NIL)
            return min;

        uint hi = high(key);
        uint lo = low(key);

        if (cluster[hi] != nullptr && cluster[hi]->getMax() != NIL && lo < cluster[hi]->getMax())
            return merge(hi, cluster[hi]->successor(lo));
        uint next_hi = summary->successor(hi);
        if (next_hi == NIL) {
            return NIL;
        }
        return merge(next_hi, cluster[next_hi]->getMin());
    }

    __attribute__((transaction_safe))
    uint predecessor(uint key) {
        if (k == 1) {
            if (key == 1 && min == 0)
                return 0;
            return NIL;
        }
        if (empty() || key <= min) {
            return NIL;
        }
        if (key > max)
            return max;

        uint hi = high(key);
        uint lo = low(key);

        if (cluster[hi]->getMin() != NIL && lo > cluster[hi]->getMin())
            return merge(hi, cluster[hi]->predecessor(lo));
        uint prev_hi = summary->predecessor(hi);
        if (prev_hi == NIL) {
            if (min != NIL && key > min)
                return min;
            return NIL;
        }
        return merge(prev_hi, cluster[prev_hi]->getMax());
    }

    void print() {
        auto i = 0;
        if (_lookup(0)) cout << 0 << endl;
        while(1) {
            i = successor(i);
            if (i == NIL) return;
            cout << i << endl;
        }
    }

    void printR() {
        auto i = UINT32_MAX;
        while(1) {
            i = predecessor(i);
            if (i == UINT32_MAX) return;
            cout << i << endl;
            if (i == 0) return;

        }
    }

private:
    __attribute__((transaction_safe))
    inline uint high(uint key) {
        return key >> (k >> 1);
    }

    __attribute__((transaction_safe))
    inline uint low(uint key) {
        return key & ((1UL << (k >> 1)) - 1);
    }

    __attribute__((transaction_safe))
    inline uint merge(uint high, uint low) {
        return (high << (k >> 1)) + low;
    }

};

template <uint k>
class vEB_stm : public vEB<k> {
private:
public:
    bool lookup(uint key) {
        bool result;
        __transaction_atomic {
                result = _lookup(key);
        }
        return result;
    }

    void insert(uint key) {
        __transaction_atomic {
                _insert(key);
        }
    }

    void remove(uint key) {
        __transaction_atomic {
                _remove(key);
        }
    }
};

template <uint k>
class vEB_lock : public vEB<k> {
private:
    mutable mutex m;
public:
    bool lookup(uint key) {
        bool result;
        m.lock();
        result = _lookup(key);
        m.unlock();
        return result;
    }

    void insert(uint key) {
        m.lock();
        _insert(key);
        m.unlock();
    }

    void remove(uint key) {
        m.lock();
        _remove(key);
        m.unlock();
    }
};

#endif //VEB_V2_VEB_H
