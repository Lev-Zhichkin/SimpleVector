#pragma once

#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <utility>

#include "array_ptr.h"



class ReserveProxyObj {

public:

    ReserveProxyObj(const size_t capacity_to_reserve)
        :bigchonk(capacity_to_reserve)
    {
    }

    size_t GetSize() { return bigchonk; }
private:
    size_t bigchonk;
};

ReserveProxyObj Reserve(const size_t capacity_to_reserve) {
    ReserveProxyObj bigchonkiscoming(capacity_to_reserve);
    return bigchonkiscoming;
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : size_(size), capacity_(size)
    {
        ArrayPtr<Type> arr_ptr(size);
        std::fill(&arr_ptr[0], &arr_ptr[size], 0);
        vec_arr_ptr_.swap(arr_ptr);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : size_(size), capacity_(size)
    {
        ArrayPtr<Type> arr_ptr(size);
        std::fill(&arr_ptr[0], &arr_ptr[size], value);
        vec_arr_ptr_.swap(arr_ptr);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()), capacity_(init.size())
    {
        ArrayPtr<Type> arr_ptr(init.size());
        std::copy(init.begin(), init.end(), arr_ptr.Get());
        //int i = 0;
        //for (auto a = init.begin(); a != init.begin(); ++a) {
        //    auto num = *a;
        //    arr_ptr[i] = num;
        //    ++i;
        //}
        vec_arr_ptr_.swap(arr_ptr);
    }

    SimpleVector(const SimpleVector& other)
        : vec_arr_ptr_(other.size_)
        , size_(other.size_)
        , capacity_(other.size_)
    {
        std::copy(other.begin(), other.end(), vec_arr_ptr_.Get());
    }

    SimpleVector(SimpleVector&& other) {            //
        vec_arr_ptr_ = std::move(other.vec_arr_ptr_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    void Reserve(size_t capacity_to_reserve) {   //
        if (capacity_to_reserve > capacity_) {
            SimpleVector<Type> tmp_items(capacity_to_reserve);
            std::move(std::make_move_iterator(cbegin()), std::make_move_iterator(cend()), tmp_items.begin());
            tmp_items.size_ = size_;
            tmp_items.capacity_ = capacity_to_reserve;
            swap(tmp_items);
        }
    }

    SimpleVector(ReserveProxyObj obj) {
        SimpleVector<Type> vec;
        vec.Reserve(obj.GetSize());
        swap(vec);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (&rhs == this) {
            return *this;
        }
        SimpleVector SV(rhs);
        this->swap(SV);
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {     //
        if (&rhs == this) {
            return *this;
        }
        vec_arr_ptr_ = std::move(rhs.vec_arr_ptr_);
        size_ = std::exchange(rhs.size_, 0);
        capacity_ = std::exchange(rhs.capacity_, 0);
        return *this;
    }

    void PushBack(const Type& item) {
        // Напишите тело самостоятельно
        size_t new_capacity;
        if (size_ == capacity_) {
            if (size_ == 0) {
                new_capacity = 1;
            }
            else {
                new_capacity = size_ * 2;
            }
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(&vec_arr_ptr_[0], &vec_arr_ptr_[size_], &tmp[0]);
            vec_arr_ptr_.swap(tmp);
            capacity_ = new_capacity;
        }
        vec_arr_ptr_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) { //
        if (capacity_ == 0) {
            SimpleVector<Type> vec(1);
            vec[0] = std::move(item);
            *this = std::move(vec);
        }
        else if (size_ == capacity_) {
            SimpleVector<Type> vec(2 * size_);
            std::move(std::make_move_iterator(begin()), std::make_move_iterator(end()), vec.begin());
            vec.size_ = size_ + 1;
            vec[size_] = std::move(item);
            *this = std::move(vec);
        }
        else {
            vec_arr_ptr_[size_] = std::move(item);
            ++size_;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t step = std::distance(cbegin(), pos);
        if (capacity_ == 0) {
            SimpleVector<Type> tmp_items(1);
            swap(tmp_items);
        }
        else {
            if (size_ == capacity_) {
                SimpleVector<Type> tmp_items(size_ * 2);
                std::copy(cbegin(), pos, tmp_items.begin());
                std::copy_backward(pos, cend(), &tmp_items[step + 1]);
                tmp_items.size_ = size_ + 1;
                swap(tmp_items);
            }
            else {
                std::copy_backward(pos, cend(), &vec_arr_ptr_[step + 1]);
                ++size_;
            }
        }
        vec_arr_ptr_[step] = value;
        return &vec_arr_ptr_[step];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        auto* p = const_cast<Iterator>(pos);
        size_t step = std::distance(begin(), p);
        if (capacity_ == 0) {
            SimpleVector<Type> tmp_items(1);
            tmp_items[step] = std::move(value);
            *this = std::move(tmp_items);
        }
        else if (size_ == capacity_) {
            SimpleVector<Type> tmp_items(2 * size_);
            std::move(std::make_move_iterator(begin()), std::make_move_iterator(p), tmp_items.begin());
            std::move_backward(std::make_move_iterator(p), std::make_move_iterator(end()), &tmp_items[step + 1]);
            tmp_items.size_ = size_ + 1;
            tmp_items[step] = std::move(value);
            *this = std::move(tmp_items);
        }
        else {
            for (size_t i = (size_); i != step; --i) {
                vec_arr_ptr_[i] = std::move(vec_arr_ptr_[i - 1]);
            }
            ++size_;
            vec_arr_ptr_[step] = std::move(value);
        }
        return &vec_arr_ptr_[step];
    } 

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {      
        auto* p = const_cast<Iterator>(pos);
        size_t step = std::distance(begin(), p);
        std::move(p + 1, end(), &vec_arr_ptr_[step]);
        --size_;
        return &vec_arr_ptr_[step];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        vec_arr_ptr_.swap(other.vec_arr_ptr_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return !size_;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return vec_arr_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return vec_arr_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) { throw std::out_of_range("Out of range"); }
        return vec_arr_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) { throw std::out_of_range("Out of range"); }
        return vec_arr_ptr_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        std::fill(&vec_arr_ptr_[0], &vec_arr_ptr_[size_], 0);
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {             //
        if (new_size == size_) { return; }
        if (new_size < size_) {
            size_ = new_size;
        }
        if (new_size > size_ && new_size < capacity_) {
            size_ = new_size;
        }
        if (new_size > capacity_) {
            ArrayPtr<Type> arr_ptr(new_size);
            std::move(std::make_move_iterator(begin()), std::make_move_iterator(end()), arr_ptr.Get());
            for (auto it = &arr_ptr[size_]; it != &arr_ptr[new_size]; ++it) {
                *(it) = std::move(Type{});
            }
            vec_arr_ptr_.swap(arr_ptr);
            size_ = new_size;
            capacity_ = new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return &vec_arr_ptr_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return &vec_arr_ptr_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return &vec_arr_ptr_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return &vec_arr_ptr_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return &vec_arr_ptr_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return &vec_arr_ptr_[size_];
    }

private:
    ArrayPtr<Type> vec_arr_ptr_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs > lhs);
}
