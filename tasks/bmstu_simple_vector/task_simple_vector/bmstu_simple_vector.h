#include <ostream>
#include <stdexcept>
#include <utility>
#include "array_ptr.h"

namespace bmstu
{
template <typename T>
class simple_vector
{
   public:
	class iterator
	{
	   public:
		using iterator_category = std::contiguous_iterator_tag;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using difference_type = std::ptrdiff_t;

		iterator() = default;

		// Создание копии итератора
		iterator(const iterator& other) = default;

		iterator(std::nullptr_t) noexcept : ptr_(nullptr) {}

		// Конструктор переноса
		iterator(iterator&& other) noexcept = default;

		// Явное создание из сырого указателя
		explicit iterator(pointer ptr) : ptr_(ptr) {}

		// Получение ссылки на значение по адресу
		reference operator*() const { return *ptr_; }

		// Доступ к полям и методам объекта
		pointer operator->() const { return ptr_; }

		// Извлечение сырого указателя для поддержки legacy-кода
		friend pointer to_address(const iterator& it) noexcept
		{
			return it.ptr_;
		}
		auto operator<=>(const iterator& other) const = default;

		// Присваивание с копированием состояния
		iterator& operator=(const iterator& other) = default;

		// Присваивание с переносом состояния
		iterator& operator=(iterator&& other) = default;

#pragma region Operators
		iterator& operator++()
		{
			++ptr_;
			return *this;
		}

		iterator& operator--()
		{
			--ptr_;
			return *this;
		}

		iterator operator++(int)
		{
			iterator temp = *this;
			++ptr_;
			return temp;
		}

		iterator operator--(int)
		{
			iterator temp = *this;
			--ptr_;
			return temp;
		}

		// Проверка итератора на валидность (указатель не null)
		explicit operator bool() const { return ptr_ != nullptr; }

		// Операции проверки на равенство
		friend bool operator==(const iterator& lhs, const iterator& rhs)
		{
			return lhs.ptr_ == rhs.ptr_;
		}

		friend bool operator==(const iterator& lhs, std::nullptr_t)
		{
			return lhs.ptr_ == nullptr;
		}

		iterator& operator=(std::nullptr_t) noexcept
		{
			ptr_ = nullptr;
			return *this;
		}

		friend bool operator==(std::nullptr_t, const iterator& rhs)
		{
			return rhs.ptr_ == nullptr;
		}

		friend bool operator!=(const iterator& lhs, const iterator& rhs)
		{
			return lhs.ptr_ != rhs.ptr_;
		}

		iterator operator+(const difference_type& n) const noexcept
		{
			return iterator(ptr_ + n);
		}

		iterator operator+=(const difference_type& n) noexcept
		{
			ptr_ += n;
			return *this;
		}

		iterator operator-(const difference_type& n) const noexcept
		{
			return iterator(ptr_ - n);
		}

		iterator operator-=(const difference_type& n) noexcept
		{
			ptr_ -= n;
			return *this;
		}

		// Вычисление дистанции в шагах между двумя итераторами
		friend difference_type operator-(const iterator& end,
										 const iterator& begin) noexcept
		{
			return end.ptr_ - begin.ptr_;
		}

#pragma endregion
	   private:
		pointer ptr_ = nullptr;
	};

	using const_iterator = iterator;

	// Базовый конструктор
	simple_vector() noexcept = default;

	// Стандартный деструктор
	~simple_vector() = default;

	// Инициализация элементов из списка {...}
	simple_vector(std::initializer_list<T> init) noexcept
		: size_(init.size()), capacity_(init.size())
	{
		if (size_ > 0)
		{
			data_ = array_ptr<T>(size_);
			size_t i = 0;

			for (auto it = init.begin(); it != init.end(); ++it)
				data_.get()[i++] = *it;
		}
	}

	// Создание на основе существующего объекта (копирование)
	simple_vector(const simple_vector& other)
		: size_(other.size_), capacity_(other.capacity_)
	{
		if (size_ > 0)
		{
			data_ = array_ptr<T>(size_);
			for (size_t i = 0; i < size_; ++i)
				data_[i] = other.data_[i];
		}
	}
	// Перенос данных из другого объекта
	// Текущий контейнер пуст, делаем обмен ресурсами с other
	simple_vector(simple_vector&& other) noexcept { swap(other); }

	// Присваивание через копирование (с защитой от самоприсваивания)
	simple_vector& operator=(const simple_vector& other)
	{
		if (this != &other)
		{
			// Локальная копия для безопасного обмена
			simple_vector temp(other);
			swap(temp);
		}
		return *this;
	}

	// Присваивание через перенос (забираем данные себе)
	simple_vector& operator=(simple_vector&& dying) noexcept
	{
		if (this != &dying)
		{
			data_ = std::move(dying.data_);

			size_ = dying.size_;
			capacity_ = dying.capacity_;

			dying.size_ = 0;
			dying.capacity_ = 0;
		}
		return *this;
	}

	// Создание вектора с заданным количеством элементов
	simple_vector(size_t size, const T& value = T{})
		: size_(size), capacity_(size)
	{
		if (size_ > 0)
		{
			data_ = array_ptr<T>(size_);

			for (size_t i = 0; i < size_; ++i)
				data_.get()[i] = value;
		}
	}

	// Итератор на начало данных
	iterator begin() noexcept { return iterator(data_.get()); }

	// Итератор за границу последнего элемента
	iterator end() noexcept { return iterator(data_.get() + size_); }

	// Константный итератор на начало данных
	const_iterator begin() const noexcept { return iterator(data_.get()); }

	// Константный итератор за границу последнего элемента
	const_iterator end() const noexcept
	{
		return iterator(data_.get() + size_);
	}

	// Прямой доступ к элементу по индексу (без проверок)
	typename iterator::reference operator[](size_t index) noexcept
	{
		return data_[index];
	}

	// Прямой доступ к элементу по индексу только для чтения
	const typename const_iterator::reference operator[](
		size_t index) const noexcept
	{
		return data_.get()[index];
	}

	typename iterator::reference at(size_t index)
	{
		if (index >= size_)
			throw std::out_of_range("Index out of range");
		return data_[index];
	}

	typename const_iterator::reference at(size_t index) const
	{
		if (index >= size_)
			throw std::out_of_range("Index out of range");
		return data_[index];
	}

	size_t size() const noexcept { return size_; }

	size_t capacity() const noexcept { return capacity_; }

	// Обмен внутренними буферами и параметрами с другим вектором
	void swap(simple_vector& other) noexcept
	{
		data_.swap(other.data_);
		std::swap(size_, other.size_);
		std::swap(capacity_, other.capacity_);
	}

	// Адаптация стандартного std::swap для нашего класса
	friend void swap(simple_vector& lhs, simple_vector& rhs) noexcept
	{
		lhs.swap(rhs);
	}

	// Расширение выделенной памяти (если требуется)
	void reserve(size_t new_cap)
	{
		if (new_cap <= capacity_)
			return;

		array_ptr<T> new_data(new_cap);

		for (size_t i = 0; i < size_; ++i)
		{
			new_data[i] = std::move(data_[i]);
		}

		data_.swap(new_data);
		capacity_ = new_cap;
	}

	// Изменение логического размера массива
	void resize(size_t new_size)
	{
		if (new_size > capacity_)
		{
			size_t new_cap = std::max(new_size, capacity_ * 2);
			reserve(new_cap);
		}

		if (new_size > size_)
		{
			for (size_t i = size_; i < new_size; ++i)
				data_[i] = T{};
		}
		size_ = new_size;
	}

	void sort() {}

	// Вставка с переносом (rvalue)
	iterator insert(const_iterator where, T&& value)
	{
		size_t index = where - begin();
		T temp = std::move(value);

		if (size_ == capacity_)
		{
			size_t new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
			reserve(new_cap);
		}

		for (size_t i = size_; i > index; --i)
			data_[i] = std::move(data_[i - 1]);

		data_[index] = std::move(temp);
		++size_;

		return begin() + index;
	}

	// Вставка с копированием (lvalue)
	iterator insert(const_iterator where, const T& value)
	{
		size_t index = where - begin();

		T temp = value;

		if (size_ == capacity_)
		{
			size_t new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
			reserve(new_cap);
		}

		for (size_t i = size_; i > index; --i)
			data_[i] = std::move(data_[i - 1]);

		data_[index] = std::move(temp);
		++size_;

		return begin() + index;
	}

	// Добавление в конец (использует семантику перемещения)
	void push_back(T&& value)
	{
		insert(end(), std::move(value));

		// if (size_ == capacity_)
		// {
		//  T temp = std::move(value);
		//  size_t new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
		//  reserve(new_cap);
		//  data_[size_] = std::move(temp);
		// }
		// else
		// {
		//  data_[size_] = std::move(value);
		// }
		// ++size_;
	}

	void clear() noexcept { size_ = 0; }

	// Добавление в конец (копирует переданное значение)
	void push_back(const T& value) { insert(end(), value); }

	bool empty() const noexcept { return size_ == 0; }

	// Отбрасывание последнего элемента
	void pop_back()
	{
		if (size_ > 0)
			--size_;
	}

	friend bool operator==(const simple_vector& lhs, const simple_vector& rhs)
	{
		if (lhs.size() != rhs.size())
			return false;

		for (size_t i = 0; i < lhs.size(); ++i)
		{
			if (lhs[i] != rhs[i])
				return false;
		}

		return true;
	}

	friend bool operator!=(const simple_vector& lhs, const simple_vector& rhs)
	{
		if (lhs.size() != rhs.size())
			return true;

		for (size_t i = 0; i < lhs.size(); ++i)
		{
			if (lhs[i] != rhs[i])
				return true;
		}
		return false;
	}

	// Трехстороннее сравнение (C++20 spaceship operator)
	friend auto operator<=>(const simple_vector& lhs, const simple_vector& rhs)
	{
		if (alphabet_compare(lhs, rhs))
			return -1;

		if (alphabet_compare(rhs, lhs))
			return 1;

		return 0;
	}

	// Форматированный вывод элементов в поток (например, std::cout)
	friend std::ostream& operator<<(std::ostream& os, const simple_vector& vec)
	{
		os << "[";
		for (size_t i = 0; i < vec.size(); ++i)
		{
			if (i > 0)
			{
				os << ", ";
			}
			os << vec[i];
		}
		os << "]";
		return os;
	}
	// Удаление элемента по переданному итератору со сдвигом хвоста
	iterator erase(iterator where)
	{
		size_t index = where - begin();
		for (size_t i = index; i < size_ - 1; ++i)
			data_[i] = data_[i + 1];

		--size_;
		return begin() + index;
	}

   private:
	static bool alphabet_compare(const simple_vector<T>& lhs,
								 const simple_vector<T>& rhs)
	{
		size_t min_size = (lhs.size() < rhs.size()) ? lhs.size() : rhs.size();

		for (size_t i = 0; i < min_size; ++i)
		{
			// Если левый элемент меньше, вся левая часть считается меньшей
			if (lhs[i] < rhs[i])
				return true;

			// Если правый элемент меньше, возвращаем false
			if (rhs[i] < lhs[i])
				return false;
		}

		// Если общая часть совпала, сравниваем длину массивов
		return lhs.size() < rhs.size();
	}
	array_ptr<T> data_;
	size_t size_ = 0;
	size_t capacity_ = 0;
};
}  // namespace bmstu