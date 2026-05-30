#pragma once

#include <cstdint>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>

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
	simple_basic_string() : ptr_(new T[1]{T(0)}), size_(0) {}

	simple_basic_string(size_t size) : ptr_(new T[size + 1]), size_(size)
	{
		for (size_t i = 0; i < size_; ++i)
		{
			ptr_[i] = T(' ');
		}
		ptr_[size_] = T(0);
	}

	simple_basic_string(std::initializer_list<T> il)
		: ptr_(new T[il.size() + 1]), size_(il.size())
	{
		size_t i = 0;
		for (const T& x : il)
		{
			ptr_[i++] = x;
		}
		ptr_[size_] = T(0);
	}

	simple_basic_string(const T* c_str)
	{
		size_ = strlen_(c_str);
		ptr_ = new T[size_ + 1];
		for (size_t i = 0; i < size_; ++i)
		{
			ptr_[i] = c_str[i];
		}
		ptr_[size_] = T(0);
	}

	simple_basic_string(const simple_basic_string& other)
		: ptr_(new T[other.size_ + 1]), size_(other.size_)
	{
		for (size_t i = 0; i < size_; ++i)
		{
			ptr_[i] = other.ptr_[i];
		}
		ptr_[size_] = T(0);
	}

	simple_basic_string(simple_basic_string&& other) noexcept
		: ptr_(other.ptr_), size_(other.size_)
	{
		other.ptr_ = new T[1]{T(0)};
		other.size_ = 0;
	}

	~simple_basic_string() { delete[] ptr_; }

	const T* c_str() const { return ptr_; }
	T* data() { return ptr_; }
	const T* data() const { return ptr_; }
	size_t size() const { return size_; }

	simple_basic_string& operator=(const simple_basic_string& other)
	{
		if (this == &other)
		{
			return *this;
		}

		T* new_ptr = new T[other.size_ + 1];
		for (size_t i = 0; i < other.size_; ++i)
		{
			new_ptr[i] = other.ptr_[i];
		}
		new_ptr[other.size_] = T(0);

		delete[] ptr_;
		ptr_ = new_ptr;
		size_ = other.size_;
		return *this;
	}

	simple_basic_string& operator=(simple_basic_string&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		delete[] ptr_;
		ptr_ = other.ptr_;
		size_ = other.size_;

		other.ptr_ = new T[1]{T(0)};
		other.size_ = 0;
		return *this;
	}

	simple_basic_string& operator=(const T* c_str)
	{
		size_t new_size = strlen_(c_str);
		T* new_ptr = new T[new_size + 1];
		for (size_t i = 0; i < new_size; ++i)
		{
			new_ptr[i] = c_str[i];
		}
		new_ptr[new_size] = T(0);

		delete[] ptr_;
		ptr_ = new_ptr;
		size_ = new_size;
		return *this;
	}

	simple_basic_string& operator+=(const simple_basic_string& other)
	{
		size_t new_size = size_ + other.size_;
		T* new_ptr = new T[new_size + 1];

		for (size_t i = 0; i < size_; ++i)
		{
			new_ptr[i] = ptr_[i];
		}
		for (size_t i = 0; i < other.size_; ++i)
		{
			new_ptr[size_ + i] = other.ptr_[i];
		}
		new_ptr[new_size] = T(0);

		delete[] ptr_;
		ptr_ = new_ptr;
		size_ = new_size;
		return *this;
	}

	simple_basic_string& operator+=(T symbol)
	{
		size_t new_size = size_ + 1;
		T* new_ptr = new T[new_size + 1];

		for (size_t i = 0; i < size_; ++i)
		{
			new_ptr[i] = ptr_[i];
		}
		new_ptr[size_] = symbol;
		new_ptr[new_size] = T(0);

		delete[] ptr_;
		ptr_ = new_ptr;
		size_ = new_size;
		return *this;
	}

	friend simple_basic_string operator+(const simple_basic_string& left,
										 const simple_basic_string& right)
	{
		simple_basic_string result(left);
		result += right;
		return result;
	}

	friend std::basic_ostream<T, std::char_traits<T>>& operator<<(
		std::basic_ostream<T, std::char_traits<T>>& os,
		const simple_basic_string& obj)
	{
		if (obj.size_ > 0)
		{
			os.write(obj.ptr_, static_cast<std::streamsize>(obj.size_));
		}
		return os;
	}

	friend std::basic_istream<T, std::char_traits<T>>& operator>>(
		std::basic_istream<T, std::char_traits<T>>& is,
		simple_basic_string& obj)
	{
		obj = simple_basic_string();
		T ch;
		while (is.get(ch))
		{
			obj += ch;
		}
		return is;
	}

	T& operator[](size_t index) noexcept { return ptr_[index]; }
	const T& operator[](size_t index) const noexcept { return ptr_[index]; }

	T& at(size_t index)
	{
		if (index >= size_)
		{
			throw std::out_of_range("Wrong index");
		}
		return ptr_[index];
	}

	const T& at(size_t index) const
	{
		if (index >= size_)
		{
			throw std::out_of_range("Wrong index");
		}
		return ptr_[index];
	}

   private:
	static size_t strlen_(const T* str)
	{
		size_t len = 0;
		while (str != nullptr && str[len] != T(0))
		{
			++len;
		}
		return len;
	}

	T* ptr_ = nullptr;
	size_t size_ = 0;
};

inline void append_utf8(std::string& s, uint32_t cp)
{
	if (cp <= 0x7F)
	{
		s += static_cast<char>(cp);
	}
	else if (cp <= 0x7FF)
	{
		s += static_cast<char>(0xC0 | (cp >> 6));
		s += static_cast<char>(0x80 | (cp & 0x3F));
	}
	else if (cp <= 0xFFFF)
	{
		s += static_cast<char>(0xE0 | (cp >> 12));
		s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
		s += static_cast<char>(0x80 | (cp & 0x3F));
	}
	else
	{
		s += static_cast<char>(0xF0 | (cp >> 18));
		s += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
		s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
		s += static_cast<char>(0x80 | (cp & 0x3F));
	}
}

inline std::string utf8_from_wstring(const wstring& ws)
{
	std::string result;
	size_t i = 0;

	while (i < ws.size())
	{
		uint32_t code_point = 0;

		if (sizeof(wchar_t) == 2)
		{
			uint16_t lead = static_cast<uint16_t>(ws[i]);

			if (lead >= 0xD800 && lead <= 0xDBFF && i + 1 < ws.size())
			{
				uint16_t trail = static_cast<uint16_t>(ws[i + 1]);

				if (trail >= 0xDC00 && trail <= 0xDFFF)
				{
					code_point =
						0x10000 + (((lead - 0xD800) << 10) | (trail - 0xDC00));
					i += 2;
				}
				else
				{
					code_point = lead;
					++i;
				}
			}
			else
			{
				code_point = lead;
				++i;
			}
		}
		else
		{
			code_point = static_cast<uint32_t>(ws[i]);
			++i;
		}

		append_utf8(result, code_point);
	}

	return result;
}

inline wstring wstring_from_utf8(const std::string& s)
{
	wstring result;
	size_t i = 0;

	while (i < s.size())
	{
		unsigned char c = static_cast<unsigned char>(s[i]);
		uint32_t code_point = 0;

		if ((c & 0x80) == 0)
		{
			code_point = c;
			++i;
		}
		else if ((c & 0xE0) == 0xC0)
		{
			if (i + 1 >= s.size())
				break;

			code_point = ((c & 0x1F) << 6) |
						 (static_cast<unsigned char>(s[i + 1]) & 0x3F);
			i += 2;
		}
		else if ((c & 0xF0) == 0xE0)
		{
			if (i + 2 >= s.size())
				break;

			code_point = ((c & 0x0F) << 12) |
						 ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6) |
						 (static_cast<unsigned char>(s[i + 2]) & 0x3F);
			i += 3;
		}
		else if ((c & 0xF8) == 0xF0)
		{
			if (i + 3 >= s.size())
				break;

			code_point = ((c & 0x07) << 18) |
						 ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12) |
						 ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6) |
						 (static_cast<unsigned char>(s[i + 3]) & 0x3F);
			i += 4;
		}
		else
		{
			break;
		}

		if (sizeof(wchar_t) == 2 && code_point > 0xFFFF)
		{
			code_point -= 0x10000;
			wchar_t high = static_cast<wchar_t>(0xD800 + (code_point >> 10));
			wchar_t low = static_cast<wchar_t>(0xDC00 + (code_point & 0x3FF));
			result += high;
			result += low;
		}
		else
		{
			result += static_cast<wchar_t>(code_point);
		}
	}

	return result;
}

inline std::ostream& operator<<(std::ostream& os, const wstring& obj)
{
	os << utf8_from_wstring(obj);
	return os;
}

inline std::istream& operator>>(std::istream& is, wstring& obj)
{
	std::string bytes;
	char ch;

	while (is.get(ch))
	{
		bytes += ch;
	}

	wstring decoded = wstring_from_utf8(bytes);

	obj = wstring();
	for (size_t i = 0; i < decoded.size(); ++i)
	{
		obj += decoded[i];
	}

	return is;
}
}  // namespace bmstu