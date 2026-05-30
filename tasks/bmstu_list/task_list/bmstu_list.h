#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <ostream>
#include "abstract_iterator.h"

namespace bmstu
{
template <typename T>
class list
{
	struct node
	{
		node() = default;

		node(node* prev, const T& value, node* next)
			: next_node_(next), prev_node_(prev), value_(value)
		{
		}

		T value_;
		node* next_node_ = nullptr;
		node* prev_node_ = nullptr;
	};

   public:
	template <typename ValueType>
	struct list_iterator
		: public abstract_iterator<list_iterator<ValueType>,
								   ValueType,
								   std::bidirectional_iterator_tag>
	{
		using difference_type = typename abstract_iterator<
			list_iterator<ValueType>,
			ValueType,
			std::bidirectional_iterator_tag>::difference_type;

		node* current;

		list_iterator() : current(nullptr) {}
		list_iterator(node* node) : current(node) {}

		template <typename NonConstT>
		list_iterator(const list_iterator<NonConstT>& other)
			: current(other.current)
		{
		}

		list_iterator& operator++() override
		{
			current = current->next_node_;
			return *this;
		}
		list_iterator& operator--() override
		{
			current = current->prev_node_;
			return *this;
		}
		list_iterator operator++(int) override
		{
			list_iterator temp = *this;
			current = current->next_node_;
			return temp;
		}
		list_iterator operator--(int) override
		{
			list_iterator temp = *this;
			current = current->prev_node_;
			return temp;
		}

		list_iterator& operator+=(const difference_type& n) override
		{
			difference_type steps = n;
			if (steps >= 0)
			{
				while (steps--)
					current = current->next_node_;
			}
			else
			{
				while (steps++)
					current = current->prev_node_;
			}
			return *this;
		}
		list_iterator& operator-=(const difference_type& n) override
		{
			return *this += (-n);
		}
		list_iterator operator+(const difference_type& n) const override
		{
			list_iterator temp = *this;
			temp += n;
			return temp;
		}
		list_iterator operator-(const difference_type& n) const override
		{
			list_iterator temp = *this;
			temp -= n;
			return temp;
		}

		typename abstract_iterator<list_iterator<ValueType>,
								   ValueType,
								   std::bidirectional_iterator_tag>::reference
		operator*() const override
		{
			return current->value_;
		}

		typename abstract_iterator<list_iterator<ValueType>,
								   ValueType,
								   std::bidirectional_iterator_tag>::pointer
		operator->() const override
		{
			return &(current->value_);
		}

		bool operator==(const list_iterator& other) const override
		{
			return current == other.current;
		}
		bool operator!=(const list_iterator& other) const override
		{
			return current != other.current;
		}
		explicit operator bool() const override { return current != nullptr; }

		difference_type operator-(const list_iterator& other) const override
		{
			difference_type dist = 0;
			node* temp = other.current;

			while (temp != current && temp != nullptr)
			{
				temp = temp->next_node_;
				dist++;
			}
			if (temp == nullptr)
			{
				dist = 0;
				temp = other.current;
				while (temp != current && temp != nullptr)
				{
					temp = temp->prev_node_;
					dist--;
				}
			}
			return dist;
		}
	};

	using iterator = list_iterator<T>;
	using const_iterator = list_iterator<const T>;

	list()
	{
		head_ = new node();
		tail_ = new node();

		head_->next_node_ = tail_;
		tail_->prev_node_ = head_;

		size_ = 0;
	}

	template <typename it>
	list(it begin, it end) : list()
	{
		for (auto current = begin; current != end; ++current)
			push_back(*current);
	}

	list(std::initializer_list<T> values) : list()
	{
		for (const auto& value : values)
			push_back(value);
	}

	list(const list& other) : list()
	{
		for (const auto& value : other)
			push_back(value);
	}

	list(list&& other) : list() { this->swap(other); }

#pragma endregion
#pragma region pushs

	template <typename Type>
	void push_back(const Type& value)
	{
		insert(const_iterator(end()), value);
	}

	template <typename Type>
	void push_front(const Type& value)
	{
		insert(const_iterator(begin()), value);
	}

#pragma endregion

	bool empty() const noexcept { return (size_ == 0u); }

	~list()
	{
		clear();
		delete head_;
		delete tail_;
	}

	void clear()
	{
		node* current = head_->next_node_;

		while (current != tail_)
		{
			node* next = current->next_node_;
			delete current;
			current = next;
		}
		head_->next_node_ = tail_;
		tail_->prev_node_ = head_;
		size_ = 0;
	}

	size_t size() const { return size_; }

	void swap(list& other) noexcept
	{
		std::swap(head_, other.head_);
		std::swap(tail_, other.tail_);
		std::swap(size_, other.size_);
	}

	friend void swap(list& l, list& r) { l.swap(r); }

#pragma region iterators

	iterator begin() noexcept { return iterator{head_->next_node_}; }

	iterator end() noexcept { return iterator{tail_}; }

	const_iterator begin() const noexcept
	{
		return const_iterator{head_->next_node_};
	}

	const_iterator end() const noexcept { return const_iterator{tail_}; }

	const_iterator cbegin() const noexcept
	{
		return const_iterator{head_->next_node_};
	}

	const_iterator cend() const noexcept { return const_iterator{tail_}; }

#pragma endregion

	T operator[](size_t pos) const
	{
		auto it = begin();
		for (size_t i = 0; i < pos; ++i)
			++it;
		return *it;
	}

	T& operator[](size_t pos)
	{
		auto it = begin();
		for (size_t i = 0; i < pos; ++i)
			++it;
		return *it;
	}

	friend bool operator==(const list& l, const list& r)
	{
		if (l.size() != r.size())
			return false;

		auto it_l = l.begin();
		auto it_r = r.begin();

		while (it_l != l.end())
		{
			if (*it_l != *it_r)
				return false;

			++it_l;
			++it_r;
		}
		return true;
	}

	friend bool operator!=(const list& l, const list& r) { return !(r == l); }

	friend auto operator<=>(const list& lhs, const list& rhs)
	{
		if (lexicographical_compare_(lhs, rhs))
			return std::strong_ordering::less;
		if (lexicographical_compare_(rhs, lhs))
			return std::strong_ordering::greater;

		return std::strong_ordering::equivalent;
	}

	friend std::ostream& operator<<(std::ostream& os, const list<T>& other)
	{
		os << "{";
		bool first = true;

		for (const auto& value : other)
		{
			if (!first)
				os << ", ";
			os << value;
			first = false;
		}
		os << "}";
		return os;
	}

	iterator insert(const_iterator pos, const T& value)
	{
		node* current = pos.current;
		node* prev_node = current->prev_node_;

		node* new_node = new node(prev_node, value, current);
		prev_node->next_node_ = new_node;
		current->prev_node_ = new_node;

		++size_;

		return iterator{new_node};
	}

   private:
	static bool lexicographical_compare_(const list<T>& l, const list<T>& r)
	{
		auto it_l = l.begin();
		auto it_r = r.begin();

		while (it_l != l.end() && it_r != r.end())
		{
			if (*it_l < *it_r)
				return true;
			if (*it_r < *it_l)
				return false;

			++it_l;
			++it_r;
		}

		return (it_l == l.end() && it_r != r.end());
	}

	size_t size_ = 0;
	node* tail_ = nullptr;
	node* head_ = nullptr;
};
}  // namespace bmstu
