#pragma once

#include <algorithm>
#include <exception>
#include <iostream>
#include <utility>

namespace bmstu
{
template <typename T>
class basic_string;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template <typename T>
class basic_string
{
   private:
	static constexpr size_t SSO_CAPACITY =
		(sizeof(T*) + sizeof(size_t) + sizeof(size_t)) / sizeof(T) - 1;

	struct LongString
	{
		T* ptr;
		size_t size;
		size_t capacity;
	};

	struct ShortString
	{
		T buffer[SSO_CAPACITY + 1];
		unsigned char size;
	};

	union Data
	{
		LongString long_str;
		ShortString short_str;

		Data() : long_str{nullptr, 0, 0} {}
		~Data() {}
	};

	Data data_;
	bool is_long_;

	bool is_long() const { return is_long_; }

	T* get_ptr()
	{
		return is_long_ ? data_.long_str.ptr : data_.short_str.buffer;
	}

	const T* get_ptr() const
	{
		return is_long_ ? data_.long_str.ptr : data_.short_str.buffer;
	}

	size_t get_size() const
	{
		return is_long_ ? data_.long_str.size : data_.short_str.size;
	}

	size_t get_capacity() const
	{
		if (is_long_)
		{
			return data_.long_str.capacity;
		}
		return SSO_CAPACITY;
	}

	void set_size(size_t size)
	{
		if (is_long_)
		{
			data_.long_str.size = size;
		}
		else
		{
			data_.short_str.size = static_cast<unsigned char>(size);
		}
	}

	void set_capacity(size_t capacity)
	{
		if (is_long_)
		{
			data_.long_str.capacity = capacity;
		}
		// Для short строки capacity фиксирован
	}

	void convert_to_long_if_needed(size_t required_capacity)
	{
		if (!is_long_ && required_capacity > SSO_CAPACITY)
		{
			LongString long_str;
			long_str.capacity = std::max(required_capacity, get_size() * 2);
			long_str.size = get_size();
			long_str.ptr = new T[long_str.capacity + 1];

			std::copy(data_.short_str.buffer,
					  data_.short_str.buffer + long_str.size + 1, long_str.ptr);

			data_.long_str = long_str;
			is_long_ = true;
		}
	}

	void reserve_internal(size_t new_capacity)
	{
		if (new_capacity <= get_capacity())
			return;

		if (is_long_)
		{
			T* new_ptr = new T[new_capacity + 1];
			std::copy(data_.long_str.ptr,
					  data_.long_str.ptr + data_.long_str.size + 1, new_ptr);
			delete[] data_.long_str.ptr;
			data_.long_str.ptr = new_ptr;
			data_.long_str.capacity = new_capacity;
		}
		else
		{
			convert_to_long_if_needed(new_capacity);
		}
	}

   public:
	/// Конструктор по умолчанию
	basic_string() : is_long_(false)
	{
		data_.short_str.size = 0;
		data_.short_str.buffer[0] = T(0);
	}

	/// Конструктор с размером
	basic_string(size_t size) : is_long_(size > SSO_CAPACITY)
	{
		if (is_long_)
		{
			data_.long_str.capacity = size;
			data_.long_str.size = size;
			data_.long_str.ptr = new T[size + 1];
			std::fill(data_.long_str.ptr, data_.long_str.ptr + size, T(' '));
			data_.long_str.ptr[size] = T(0);
		}
		else
		{
			data_.short_str.size = static_cast<unsigned char>(size);
			std::fill(data_.short_str.buffer, data_.short_str.buffer + size,
					  T(' '));
			data_.short_str.buffer[size] = T(0);
		}
	}

	/// Конструктор с initializer_list
	basic_string(std::initializer_list<T> il)
		: is_long_(il.size() > SSO_CAPACITY)
	{
		if (is_long_)
		{
			data_.long_str.capacity = il.size();
			data_.long_str.size = il.size();
			data_.long_str.ptr = new T[il.size() + 1];
			std::copy(il.begin(), il.end(), data_.long_str.ptr);
			data_.long_str.ptr[il.size()] = T(0);
		}
		else
		{
			data_.short_str.size = static_cast<unsigned char>(il.size());
			std::copy(il.begin(), il.end(), data_.short_str.buffer);
			data_.short_str.buffer[il.size()] = T(0);
		}
	}

	/// Конструктор с C-строкой
	basic_string(const T* c_str)
	{
		size_t len = strlen_(c_str);
		is_long_ = (len > SSO_CAPACITY);

		if (is_long_)
		{
			data_.long_str.capacity = len;
			data_.long_str.size = len;
			data_.long_str.ptr = new T[len + 1];
			std::copy(c_str, c_str + len + 1, data_.long_str.ptr);
		}
		else
		{
			data_.short_str.size = static_cast<unsigned char>(len);
			std::copy(c_str, c_str + len + 1, data_.short_str.buffer);
		}
	}

	/// Конструктор копирования
	basic_string(const basic_string& other) : is_long_(other.is_long_)
	{
		if (is_long_)
		{
			data_.long_str.capacity = other.data_.long_str.size;
			data_.long_str.size = other.data_.long_str.size;
			data_.long_str.ptr = new T[data_.long_str.size + 1];
			std::copy(other.data_.long_str.ptr,
					  other.data_.long_str.ptr + data_.long_str.size + 1,
					  data_.long_str.ptr);
		}
		else
		{
			data_.short_str.size = other.data_.short_str.size;
			std::copy(other.data_.short_str.buffer,
					  other.data_.short_str.buffer + SSO_CAPACITY + 1,
					  data_.short_str.buffer);
		}
	}

	/// Перемещающий конструктор
	basic_string(basic_string&& other) noexcept : is_long_(other.is_long_)
	{
		if (is_long_)
		{
			data_.long_str = other.data_.long_str;
			other.data_.long_str.ptr = nullptr;
			other.data_.long_str.size = 0;
			other.data_.long_str.capacity = 0;
			other.is_long_ = false;
			other.data_.short_str.size = 0;
			other.data_.short_str.buffer[0] = T(0);
		}
		else
		{
			data_.short_str = other.data_.short_str;
			other.data_.short_str.size = 0;
			other.data_.short_str.buffer[0] = T(0);
		}
	}

	/// Деструктор
	~basic_string()
	{
		if (is_long_ && data_.long_str.ptr)
		{
			delete[] data_.long_str.ptr;
		}
	}

	/// Получить C-строку
	const T* c_str() const { return get_ptr(); }

	/// Получить размер
	size_t size() const { return get_size(); }

	/// Проверка использования SSO
	bool is_using_sso() const { return !is_long_; }

	/// Получить емкость
	size_t capacity() const { return get_capacity(); }

	/// Зарезервировать память
	void reserve(size_t new_capacity) { reserve_internal(new_capacity); }

	/// Изменить размер строки
	void resize(size_t new_size, T fill_char = T(' '))
	{
		if (new_size < get_size())
		{
			set_size(new_size);
			get_ptr()[new_size] = T(0);
		}
		else if (new_size > get_size())
		{
			reserve_internal(new_size);
			T* ptr = get_ptr();
			std::fill(ptr + get_size(), ptr + new_size, fill_char);
			set_size(new_size);
			ptr[new_size] = T(0);
		}
	}

	/// Очистить строку
	void clear()
	{
		set_size(0);
		get_ptr()[0] = T(0);
	}

	/// Оператор перемещающего присваивания
	basic_string& operator=(basic_string&& other) noexcept
	{
		if (this != &other)
		{
			clean_();
			is_long_ = other.is_long_;

			if (is_long_)
			{
				data_.long_str = other.data_.long_str;
				other.data_.long_str.ptr = nullptr;
				other.data_.long_str.size = 0;
				other.data_.long_str.capacity = 0;
				other.is_long_ = false;
				other.data_.short_str.size = 0;
				other.data_.short_str.buffer[0] = T(0);
			}
			else
			{
				data_.short_str = other.data_.short_str;
				other.data_.short_str.size = 0;
				other.data_.short_str.buffer[0] = T(0);
			}
		}
		return *this;
	}

	/// Оператор присваивания C-строки
	basic_string& operator=(const T* c_str)
	{
		size_t len = strlen_(c_str);

		if (len <= SSO_CAPACITY && !is_long_)
		{
			// Остаемся в short режиме
			data_.short_str.size = static_cast<unsigned char>(len);
			std::copy(c_str, c_str + len + 1, data_.short_str.buffer);
		}
		else
		{
			// Нужен long режим
			if (!is_long_)
			{
				// Конвертируем из short в long
				is_long_ = true;
				data_.long_str.capacity = len;
				data_.long_str.size = len;
				data_.long_str.ptr = new T[len + 1];
				std::copy(c_str, c_str + len + 1, data_.long_str.ptr);
			}
			else if (len <= data_.long_str.capacity)
			{
				// Используем существующий буфер
				data_.long_str.size = len;
				std::copy(c_str, c_str + len, data_.long_str.ptr);
				data_.long_str.ptr[len] = T(0);
			}
			else
			{
				// Нужен новый буфер
				delete[] data_.long_str.ptr;
				data_.long_str.capacity = len;
				data_.long_str.size = len;
				data_.long_str.ptr = new T[len + 1];
				std::copy(c_str, c_str + len + 1, data_.long_str.ptr);
			}
		}

		return *this;
	}

	/// Оператор копирующего присваивания
	basic_string& operator=(const basic_string& other)
	{
		if (this != &other)
		{
			clean_();
			is_long_ = other.is_long_;

			if (is_long_)
			{
				data_.long_str.capacity = other.data_.long_str.size;
				data_.long_str.size = other.data_.long_str.size;
				data_.long_str.ptr = new T[data_.long_str.size + 1];
				std::copy(other.data_.long_str.ptr,
						  other.data_.long_str.ptr + data_.long_str.size + 1,
						  data_.long_str.ptr);
			}
			else
			{
				data_.short_str.size = other.data_.short_str.size;
				std::copy(other.data_.short_str.buffer,
						  other.data_.short_str.buffer + SSO_CAPACITY + 1,
						  data_.short_str.buffer);
			}
		}
		return *this;
	}

	friend basic_string<T> operator+(const basic_string<T>& left,
									 const basic_string<T>& right)
	{
		basic_string<T> result;
		size_t total_size = left.size() + right.size();

		result.reserve_internal(total_size);
		result.set_size(total_size);

		T* dest = result.get_ptr();
		const T* src_left = left.get_ptr();
		const T* src_right = right.get_ptr();

		std::copy(src_left, src_left + left.size(), dest);
		std::copy(src_right, src_right + right.size(), dest + left.size());
		dest[total_size] = T(0);

		return result;
	}

	template <typename S>
	friend S& operator<<(S& os, const basic_string& obj)
	{
		os << obj.c_str();
		return os;
	}

	template <typename S>
	friend S& operator>>(S& is, basic_string& obj)
	{
		T ch;
		basic_string temp;

		while (is.get(ch))
		{
			temp += ch;
		}

		if (is)
		{
			is.putback(ch);
		}

		obj = std::move(temp);
		return is;
	}

	basic_string& operator+=(const basic_string& other)
	{
		size_t old_size = size();
		size_t other_size = other.size();
		size_t new_size = old_size + other_size;

		reserve_internal(new_size);
		set_size(new_size);

		T* ptr = get_ptr();
		const T* other_ptr = other.get_ptr();

		std::copy(other_ptr, other_ptr + other_size, ptr + old_size);
		ptr[new_size] = T(0);

		return *this;
	}

	basic_string& operator+=(T symbol)
	{
		size_t old_size = size();
		size_t new_size = old_size + 1;

		reserve_internal(new_size);
		set_size(new_size);

		T* ptr = get_ptr();
		ptr[old_size] = symbol;
		ptr[new_size] = T(0);

		return *this;
	}

	T& operator[](size_t index) noexcept { return get_ptr()[index]; }

	const T& operator[](size_t index) const noexcept
	{
		return get_ptr()[index];
	}

	T& at(size_t index)
	{
		if (index >= size())
		{
			throw std::out_of_range("Wrong index");
		}
		return get_ptr()[index];
	}

	const T& at(size_t index) const
	{
		if (index >= size())
		{
			throw std::out_of_range("Wrong index");
		}
		return get_ptr()[index];
	}

	T* data() { return get_ptr(); }

	const T* data() const { return get_ptr(); }

   private:
	static size_t strlen_(const T* str)
	{
		size_t result = 0;
		while (*str != T(0))
		{
			++result;
			++str;
		}
		return result;
	}

	void clean_()
	{
		if (is_long_ && data_.long_str.ptr)
		{
			delete[] data_.long_str.ptr;
			data_.long_str.ptr = nullptr;
			data_.long_str.size = 0;
			data_.long_str.capacity = 0;
		}
		is_long_ = false;
		data_.short_str.size = 0;
		data_.short_str.buffer[0] = T(0);
	}
};
}  // namespace bmstu
