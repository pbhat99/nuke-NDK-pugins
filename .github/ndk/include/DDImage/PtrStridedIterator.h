#ifndef DDIMAGE_PTRSTRIDEDITERATOR_H
#define DDIMAGE_PTRSTRIDEDITERATOR_H

#include <iterator>

namespace DD {
  namespace Image {

    //! Random access iterator with adjustable iteration step (stride).
    //! The iterator will skip over multiple entries when it is incremented or decremented.
    template <typename T>
    struct PtrStridedIterator
    {
      using difference_type   = std::ptrdiff_t;
      using value_type        = T;
      using pointer           = T*;
      using reference         = T&;
      using iterator_category = std::random_access_iterator_tag;

    private:
      pointer _ptr;
      difference_type _stride;

    public:
      PtrStridedIterator() : _ptr(nullptr), _stride(1) {}
      PtrStridedIterator(pointer ptr, difference_type strd = 1) : _ptr(ptr), _stride(strd) {}
      PtrStridedIterator(const PtrStridedIterator&) = default;
      PtrStridedIterator(PtrStridedIterator&&)      = default;

      ~PtrStridedIterator()                         = default;

      PtrStridedIterator& operator=(const PtrStridedIterator& other) = default;
      PtrStridedIterator& operator=(PtrStridedIterator&& other)      = default;

      pointer ptr() const
      {
        return _ptr;
      }

      difference_type stride() const
      {
        return _stride;
      }

      reference operator[](difference_type i) const
      {
        const difference_type offset = i * _stride;
        return _ptr[offset];
      }

      reference operator*() const
      {
        return *_ptr;
      }

      reference operator->() const
      {
        return *_ptr;
      }

      PtrStridedIterator& operator+=(difference_type offset)
      {
        const difference_type idx = offset * _stride;
        _ptr += idx;
        return *this;
      }

      PtrStridedIterator& operator-=(difference_type offset)
      {
        const difference_type idx = offset * _stride;
        _ptr -= idx;
        return *this;
      }

      PtrStridedIterator operator+(difference_type offset) const
      {
        const difference_type idx = offset * _stride;
        return PtrStridedIterator(_ptr + idx, _stride);
      }

      PtrStridedIterator operator-(difference_type offset) const
      {
        const difference_type idx = offset * _stride;
        return PtrStridedIterator(_ptr - idx, _stride);
      }

      difference_type operator-(const PtrStridedIterator& other) const
      {
        difference_type diff = _ptr - other._ptr;
        return diff / _stride;
      }

      PtrStridedIterator& operator++()
      {
        _ptr += _stride;
        return *this;
      }

      PtrStridedIterator operator++(int)
      {
        PtrStridedIterator old = *this;
        _ptr += _stride;
        return old;
      }

      PtrStridedIterator& operator--()
      {
        _ptr -= _stride;
        return *this;
      }

      PtrStridedIterator operator--(int)
      {
        PtrStridedIterator old = *this;
        _ptr -= _stride;
        return old;
      }

      bool operator==(const PtrStridedIterator& other) const { return _ptr == other._ptr; }
      bool operator< (const PtrStridedIterator& other) const { return _ptr <  other._ptr; }
      bool operator> (const PtrStridedIterator& other) const { return _ptr >  other._ptr; }
      bool operator!=(const PtrStridedIterator& other) const { return !operator==(other); }
      bool operator<=(const PtrStridedIterator& other) const { return !operator> (other); }
      bool operator>=(const PtrStridedIterator& other) const { return !operator< (other); }

      explicit operator bool() const { return _ptr != nullptr; }
    };

  }
}

#endif
