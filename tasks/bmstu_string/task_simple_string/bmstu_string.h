#pragma once

#include <exception>
#include <iostream>

namespace bmstu
{
template <typename T>
class simple_basic_string;

typedef simple_basic_string<char> string;
typedef simple_basic_string<wchar_t> wstring;
typedef simple_basic_string<char16_t> u16string;
typedef simple_basic_string<char32_t> u32string;

template <typename T>
class simple_basic_string
{
   public:
	/// Конструктор по умолчанию
	simple_basic_string() : ptr_(new T[1]{0}), size_(0) {}

	/// Создает строку заданной длины
	simple_basic_string(size_t size) : ptr_(new T[size + 1]), size_(size)
	{
		for (size_t i = 0; i < size; ++i)
		{
			ptr_[i] = ' ';
		}
		ptr_[size_] = 0;
	}
	/// Создание строки через initializer list
	simple_basic_string(std::initializer_list<T> il)
		: ptr_(new T[il.size() + 1]), size_(il.size())
	{
		size_t i = 0;

		for (const T& val : il)
		{
			ptr_[i] = val;
			i++;
		}
		ptr_[size_] = 0;
	}
	/// Конструктор с параметром си
	simple_basic_string(const T* c_str)
	{
		if (c_str == nullptr)
		{
			size_ = 0;
			ptr_ = new T[1]{0};
		}
		else
		{
			size_ = strlen_(c_str);
			ptr_ = new T[size_ + 1];
			for (size_t i = 0; i < size_; ++i)
			{
				ptr_[i] = c_str[i];
			}
			ptr_[size_] = 0;
		}
	}

	/// Конструктор копирования
	simple_basic_string(const simple_basic_string& other)
		: ptr_(new T[other.size_ + 1]), size_(other.size_)
	{
		for (size_t i = 0; i < size_; i++)
		{
			ptr_[i] = other.ptr_[i];
		}
		ptr_[size_] = 0;
	}

	/// Перемещающий конструктор
	simple_basic_string(simple_basic_string&& other)
		: ptr_(other.ptr_), size_(other.size_)
	{
		other.ptr_ = new T[1]{0};
		other.size_ = 0;
	}

	/// Деструктор
	~simple_basic_string() { delete[] ptr_; }

	/// Геттер на си-строку
	const T* c_str() const { return ptr_; }

	size_t size() const { return size_; }

	/// Оператор перемещающего присваивания си
	simple_basic_string& operator=(simple_basic_string&& other)
	{
		delete[] ptr_;

		// copy and swap
		ptr_ = other.ptr_;
		size_ = other.size_;

		other.size_ = 0;
		other.ptr_ = new T[1];
		other.ptr_[0] = T(0);

		return *this;
	}

	/// Оператор копирующего присваивания с строки
	simple_basic_string& operator=(const T* c_str)
	{
		simple_basic_string copy(c_str);
		std::swap(copy, *this);
		return *this;
	}

	/// Оператор копирующего присваивания (копирует other в this)
	simple_basic_string& operator=(const simple_basic_string& other)
	{
		if (this == &other)
			return *this;
		/// Очистка старой памяти
		clean_();
		size_ = other.size_;
		ptr_ = new T[size_ + 1];

		/// глубокое копирование
		for (size_t i = 0; i < size_; ++i)
		{
			ptr_[i] = other.ptr_[i];
		}

		return *this;
	}
	/// Сложение строк, перегрузка оператора+
	friend simple_basic_string<T> operator+(const simple_basic_string<T>& left,
											const simple_basic_string<T>& right)
	{
		size_t new_size = left.size() + right.size();
		simple_basic_string<T> result(new_size);
		for (size_t i = 0; i < left.size_; ++i)
		{
			result.ptr_[i] = left.ptr_[i];
		}
		for (size_t i = 0; i < right.size_; ++i)
		{
			result.ptr_[left.size_ + i] = right.ptr_[i];
		}
		result.ptr_[new_size] = 0;
		return result;
	}
	/// ввода строки в поток
	template <typename S>
	friend S& operator<<(S& os, const simple_basic_string& obj)
	{
		os << obj.c_str();
		return os;
	}
	/// оператор ввода
	template <typename S>
	friend S& operator>>(S& is, simple_basic_string& obj)
	{
		delete[] obj.ptr_;
		obj.ptr_ = new T[1]{0};
		obj.size_ = 0;
		T symbol;
		if (!(is >> symbol))
		{
			return is;
		}
		obj += symbol;
		while (is.get(symbol))
		{
			obj += symbol;
		}
		return is;
	}

	simple_basic_string& operator+=(const simple_basic_string& other)
	{
		return *this;
	}
	/// добавление буквы в конец
	simple_basic_string& operator+=(T symbol)
	{
		T* new_ptr = new T[size_ + 2];

		for (size_t i = 0; i < size_; ++i)
		{
			new_ptr[i] = ptr_[i];
		}

		new_ptr[size_] = symbol;
		new_ptr[size_ + 1] = 0;
		delete[] ptr_;
		ptr_ = new_ptr;
		size_ += 1;

		return *this;
	}

	T& operator[](size_t index) noexcept { return *(ptr_ + index); }

	T& at(size_t index) { throw std::out_of_range("Wrong index"); }

	T* data() { return ptr_; }

   private:
	static size_t strlen_(const T* str)
	{
		size_t i = 0;
		while (str[i])
		{
			i++;
		}
		return i;
	}

	void clean_() {}

	T* ptr_ = nullptr;
	size_t size_;
};
}  // namespace bmstu