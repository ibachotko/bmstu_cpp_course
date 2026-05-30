#pragma once

#include <exception>
#include <iostream>

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
	};

	Data data_;
	bool is_long_;

	T* get_ptr()
	{
		if (is_long_)
			return data_.long_str.ptr;
		else
			return data_.short_str.buffer;
	}

	const T* get_ptr() const
	{
		if (is_long_)
			return data_.long_str.ptr;
		else
			return data_.short_str.buffer;
	}

	size_t get_size() const
	{
		if (is_long_)
			return data_.long_str.size;
		else
			return data_.short_str.size;
	}

	size_t get_capacity() const
	{
		if (is_long_)
			return data_.long_str.capacity;
		else
			return SSO_CAPACITY;
	}

   public:
	basic_string()
	{
		is_long_ = false;
		data_.short_str.size = 0;
		data_.short_str.buffer[0] = T(0);
	}

	basic_string(size_t size)
	{
		if (size <= SSO_CAPACITY)
		{
			// КОРОТКИЙ РЕЖИМ (СТЕК)
			is_long_ = false;
			data_.short_str.size = static_cast<unsigned char>(size);

			for (size_t i = 0; i < size; ++i)
				data_.short_str.buffer[i] = T(' ');
			data_.short_str.buffer[size] = T(0);
		}
		else
		{
			// ДЛИННЫЙ РЕЖИМ (КУЧА)
			is_long_ = true;
			data_.long_str.size = size;
			data_.long_str.capacity = size;
			data_.long_str.ptr = new T[size + 1];

			for (size_t i = 0; i < size; ++i)
				data_.long_str.ptr[i] = T(' ');
			data_.long_str.ptr[size] = T(0);
		}
	}
	bool is_long() const { return is_long_; }
	basic_string(std::initializer_list<T> il)
	{
		size_t len = il.size();

		if (len <= SSO_CAPACITY)
		{
			// КОРОТКИЙ РЕЖИМ (СТЕК)

			is_long_ = false;
			data_.short_str.size = static_cast<unsigned char>(len);

			size_t i = 0;
			for (const T& symbol : il)
				data_.short_str.buffer[i++] = symbol;
			data_.short_str.buffer[len] = T(0);
		}
		else
		{
			// ДЛИННЫЙ РЕЖИМ (КУЧА)

			is_long_ = true;
			data_.long_str.size = len;
			data_.long_str.capacity = len;

			data_.long_str.ptr = new T[len + 1];

			size_t i = 0;
			for (const T& symbol : il)
				data_.long_str.ptr[i] = symbol;
			data_.long_str.ptr[len] = T(0);
		}
	}

	/// Конструктор от Си-строки
	basic_string(const T* c_str)
	{
		if (!c_str)
		{
			is_long_ = false;
			data_.short_str.size = 0;
			data_.short_str.buffer[0] = T(0);
			return;
		}

		size_t len = strlen_(c_str);

		if (len <= SSO_CAPACITY)
		{
			// КОРОТКИЙ РЕЖИМ (СТЕК)

			is_long_ = false;
			data_.short_str.size = static_cast<unsigned char>(len);

			for (size_t i = 0; i < len; ++i)
				data_.short_str.buffer[i] = c_str[i];
			data_.short_str.buffer[len] = T(0);
		}
		else
		{
			// ДЛИННЫЙ РЕЖИМ (КУЧА)

			is_long_ = true;
			data_.long_str.size = len;
			data_.long_str.capacity = len;

			data_.long_str.ptr = new T[len + 1];

			for (size_t i = 0; i < len; ++i)
			{
				data_.long_str.ptr[i] = c_str[i];
			}
			data_.long_str.ptr[len] = T(0);
		}
	}

	/// Конструктор копирования
	basic_string(const basic_string& other)
	{
		is_long_ = other.is_long_;
		if (!other.is_long_)
		{
			// КОРОТКИЙ РЕЖИМ (СТЕК)

			data_.short_str.size = other.data_.short_str.size;

			for (size_t i = 0; i < data_.short_str.size; ++i)
				data_.short_str.buffer[i] = other.data_.short_str.buffer[i];
			data_.short_str.buffer[data_.short_str.size] = T(0);
		}
		else
		{
			// ДЛИННЫЙ РЕЖИМ (КУЧА)

			data_.long_str.size = other.data_.long_str.size;
			data_.long_str.capacity = other.data_.long_str.capacity;
			data_.long_str.ptr = new T[data_.long_str.capacity + 1];

			for (size_t i = 0; i < data_.long_str.size; ++i)
				data_.long_str.ptr[i] = other.data_.long_str.ptr[i];
			data_.long_str.ptr[data_.long_str.size] = T(0);
		}
	}

	/// Конструктор перемещения
	basic_string(basic_string&& dying) noexcept
	{
		is_long_ = dying.is_long_;

		if (!is_long_)
		{
			// КОРОТКИЙ РЕЖИМ (СТЕК)

			data_.short_str.size = dying.data_.short_str.size;

			for (size_t i = 0; i <= data_.short_str.size; ++i)
				data_.short_str.buffer[i] = dying.data_.short_str.buffer[i];

			dying.data_.short_str.size = 0;
			dying.data_.short_str.buffer[0] = T(0);
		}
		else
		{
			// ДЛИННЫЙ РЕЖИМ (КУЧА)

			data_.long_str = dying.data_.long_str;

			dying.is_long_ = false;
			dying.data_.short_str.size = 0;
			dying.data_.short_str.buffer[0] = T(0);
		}
	}

	~basic_string() { clean_(); }

	const T* c_str() const { return get_ptr(); }

	size_t size() const { return get_size(); }

	bool is_using_sso() const { return !is_long_; }

	size_t capacity() const { return get_capacity(); }

	// ОПЕРАТОР ПЕРЕМЕЩАЮЩЕГО ПРИСВАИВАНИЯ
	basic_string& operator=(basic_string&& other) noexcept
	{
		if (this != &other)
		{
			clean_();

			is_long_ = other.is_long_;
			if (!is_long_)
			{
				// КОРОТКИЙ РЕЖИМ (СТЕК)

				data_.short_str.size = other.data_.short_str.size;

				for (size_t i = 0; i <= data_.short_str.size; ++i)
					data_.short_str.buffer[i] = other.data_.short_str.buffer[i];

				other.data_.short_str.size = 0;
				other.data_.short_str.buffer[0] = T(0);
			}
			else
			{
				// ДЛИННЫЙ РЕЖИМ (КУЧА)

				data_.long_str = other.data_.long_str;

				other.is_long_ = false;
				other.data_.short_str.size = 0;
				other.data_.short_str.buffer[0] = T(0);
			}
		}
		return *this;
	}

	/// Оператор присваивания из Си-строки
	basic_string& operator=(const T* c_str)
	{
		if (get_ptr() == c_str)
			return *this;

		clean_();

		size_t new_size = strlen_(c_str);
		if (new_size <= SSO_CAPACITY)
		{
			// КОРОТКИЙ РЕЖИМ (СТЕК)

			is_long_ = false;
			data_.short_str.size = new_size;
			for (size_t i = 0; i < new_size; ++i)
				data_.short_str.buffer[i] = c_str[i];
			data_.short_str.buffer[new_size] = T(0);
		}
		else
		{
			// ДЛИННЫЙ РЕЖИМ (КУЧА)

			is_long_ = true;

			data_.long_str.size = new_size;
			data_.long_str.capacity = new_size;
			data_.long_str.ptr = new T[data_.long_str.capacity + 1];

			for (size_t i = 0; i < new_size; ++i)
				data_.long_str.ptr[i] = c_str[i];
			data_.long_str.ptr[new_size] = T(0);
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

			if (!is_long_)
			{
				// КОРОТКИЙ РЕЖИМ (СТЕК)

				data_.short_str.size = other.data_.short_str.size;

				for (size_t i = 0; i <= data_.short_str.size; ++i)
					data_.short_str.buffer[i] = other.data_.short_str.buffer[i];
			}
			else
			{
				// ДЛИННЫЙ РЕЖИМ (КУЧА)

				data_.long_str.size = other.data_.long_str.size;
				data_.long_str.capacity = other.data_.long_str.capacity;
				data_.long_str.ptr = new T[data_.long_str.capacity + 1];

				for (size_t i = 0; i < data_.long_str.size; ++i)
					data_.long_str.ptr[i] = other.data_.long_str.ptr[i];
				data_.long_str.ptr[data_.long_str.size] = T(0);
			}
		}

		return *this;
	}

	/// Конкатенация строк
	friend basic_string<T> operator+(const basic_string<T>& left,
									 const basic_string<T>& right)
	{
		size_t l_len = left.size();
		size_t r_len = right.size();
		size_t total_len = l_len + r_len;

		basic_string result(total_len);

		T* res_ptr = result.get_ptr();
		const T* l_ptr = left.get_ptr();
		const T* r_ptr = right.get_ptr();

		for (size_t i = 0; i < l_len; ++i)
			res_ptr[i] = l_ptr[i];

		for (size_t i = 0; i < r_len; ++i)
			res_ptr[i + l_len] = r_ptr[i];

		return result;
	}

	template <typename S>
	friend S& operator<<(S& os, const basic_string& obj)
	{
		const T* current_ptr = obj.get_ptr();
		size_t current_size = obj.size();

		for (size_t i = 0; i < current_size; ++i)
			os << current_ptr[i];

		return os;
	}

	template <typename S>
	friend S& operator>>(S& is, basic_string& obj)
	{
		obj.clean_();

		T ch;
		while (is.get(ch))
			obj += ch;
		return is;
	}

	basic_string& operator+=(const basic_string& other)
	{
		size_t current_size = size();
		size_t other_size = other.size();

		size_t new_size = other_size + current_size;

		const T* other_ptr = other.get_ptr();

		if (new_size <= SSO_CAPACITY)
		{
			// КОРОТКИЙ РЕЖИМ (СТЕК)

			for (size_t i = 0; i < other_size; ++i)
				data_.short_str.buffer[current_size + i] = other_ptr[i];

			data_.short_str.size = static_cast<unsigned char>(new_size);
			data_.short_str.buffer[new_size] = T(0);
			is_long_ = false;
		}
		else
		{
			// ДЛИННЫЙ РЕЖИМ (КУЧА)
			T* new_ptr = new T[new_size + 1];

			const T* current_ptr = get_ptr();
			for (size_t i = 0; i < current_size; ++i)
				new_ptr[i] = current_ptr[i];

			for (size_t i = 0; i < other_size; ++i)
				new_ptr[current_size + i] = other_ptr[i];

			new_ptr[new_size] = T(0);

			clean_();
			is_long_ = true;
			data_.long_str.size = new_size;
			data_.long_str.capacity = new_size;
			data_.long_str.ptr = new_ptr;
		}

		return *this;
	}

	/// Прибавление символа
	basic_string& operator+=(T symbol)
	{
		size_t new_size = size() + 1;
		if (new_size < SSO_CAPACITY)
		{
			// КОРОТКИЙ РЕЖИМ (СТЕК)

			data_.short_str.buffer[new_size - 1] = symbol;
			data_.short_str.buffer[new_size] = T(0);
			data_.short_str.size = static_cast<unsigned char>(new_size);
			is_long_ = false;
		}
		else
		{
			// ДЛИННЫЙ РЕЖИМ (КУЧА)

			T* new_ptr = new T[new_size + 1];
			const T* current_ptr = get_ptr();

			for (size_t i = 0; i < new_size - 1; ++i)
				new_ptr[i] = current_ptr[i];
			new_ptr[new_size - 1] = symbol;
			new_ptr[new_size] = T(0);

			clean_();
			is_long_ = true;
			data_.long_str.size = new_size;
			data_.long_str.capacity = new_size;
			data_.long_str.ptr = new_ptr;
		}
		return *this;
	}

	basic_string& reverse()
	{
		size_t this_size = size();
		for (size_t i = 0; i < size() / 2; ++i)
		{
			std::swap((*this)[i], (*this)[this_size - 1 - i]);
		}

		return *this;
	}

	friend bool operator==(const basic_string<T>& left,
						   const basic_string<T>& right)
	{
		if (left.size() != right.size())
			return false;

		for (size_t i = 0; i < left.size(); ++i)
		{
			if (left[i] != right[i])
				return false;
		}

		return true;
	}
	/// Прямой и быстрый доступ по индексу
	T& operator[](size_t index) noexcept { return get_ptr()[index]; }
	const T& operator[](size_t index) const noexcept
	{
		return get_ptr()[index];
	}

	/// Безопасный доступ по индексу
	T& at(size_t index)
	{
		if (index >= size())
			std::out_of_range("Index out of range!");
		return get_ptr()[index];
	}

	T* data() { return data_; }

   private:
	static size_t strlen_(const T* str)
	{
		size_t len = 0;
		while (str != nullptr && str[len] != T(0))
			++len;
		return len;
	}

	void clean_()
	{
		if (is_long_)
			delete[] data_.long_str.ptr;

		is_long_ = false;
		data_.short_str.size = 0;
		data_.short_str.buffer[0] = T(0);
	}
};
}  // namespace bmstu