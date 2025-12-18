#pragma once

#include <exception>
#include <iostream>
#include <utility>

namespace bmstu
{
template <typename T>
class stack
{
   public:
	stack() : data_(nullptr), size_(0), capacity_(0) {}

	bool empty() const noexcept { return size_ == 0; }

	size_t size() const noexcept { return size_; }

	~stack()
	{
		clear();
		::operator delete(data_);
	}

	template <typename... Args>
	void emplace(Args&&... args)
	{
		if (size_ == capacity_)
		{
			// новая capacity
			size_t new_cap = (capacity_ == 0) ? 1 : capacity_ + 1;

			// создание нового массива data
			T* new_data = (T*)::operator new(new_cap * sizeof(T));

			// перемещение старых элементов
			for (size_t i = 0; i < size_; ++i)
			{
				// копирование данных из старого объекта
				new (new_data + i) T(std::move(data_[i]));

				// вызываем деструктор класса T, чтобы удалить объект из старой
				// data
				(data_ + i)->~T();
			}

			::operator delete(data_);

			data_ = new_data;
			capacity_ = new_cap;
		}

		new (data_ + size_) T(std::forward<Args>(args)...);
		++size_;
	}

	// copy semantics (lvalue)
	void push(const T& value)
	{
		if (size_ == capacity_)
		{
			// новый capacity
			size_t new_cap = (capacity_ == 0) ? 1 : capacity_ + 1;

			// новый массив new_data
			T* new_data = (T*)::operator new(new_cap * sizeof(T));

			// перемещение старых элементов
			for (size_t i = 0; i < size_; ++i)
			{
				// копирование данных из старого объекта
				new (new_data + i) T(std::move(data_[i]));

				// вызываем деструктор класса T, чтобы удалить объект из старой
				// data
				(data_ + i)->~T();
			}

			// очистка старого массива глупым методом ogo
			::operator delete(data_);

			data_ = new_data;
			capacity_ = new_cap;
		}

		// с помощью конструктора для типа Т присваиваем сырой памяти value
		// (copy)
		new (data_ + size_) T(value);
		++size_;
	}

	// move semantics (rvalue)
	void push(T&& value)
	{
		if (size_ == capacity_)
		{
			// новый capacity
			size_t new_cap = (capacity_ == 0) ? 1 : capacity_ + 1;

			// новый массив new_data
			T* new_data = (T*)::operator new(new_cap * sizeof(T));

			// перемещение старых элементов
			for (size_t i = 0; i < size_; ++i)
			{
				// копирование данных из старого объекта
				new (new_data + i) T(std::move(data_[i]));

				// вызываем деструктор класса T, чтобы удалить объект из старой
				// data
				(data_ + i)->~T();
			}

			// очистка старого массива глупым методом
			::operator delete(data_);

			data_ = new_data;
			capacity_ = new_cap;
		}

		// с помощью move записываем новым элементом value
		new (data_ + size_) T(std::move(value));
		++size_;
	}

	void clear() noexcept
	{
		while (size_ > 0)
		{
			--size_;
			(data_ + size_)->~T();
		}
	}

	void pop()
	{
		if (empty())
			throw std::underflow_error("Stack is empty!");
		--size_;
		(data_ + size_)->~T();
	}

	T& top()
	{
		if (empty())
			throw std::underflow_error("Stack is empty!");
		return data_[size_ - 1];
	}

	const T& top() const
	{
		if (empty())
			throw std::underflow_error("Stack is empty!");
		return data_[size_ - 1];
	}

   private:
	T* data_;
	size_t size_;
	size_t capacity_;
};
}  // namespace bmstu