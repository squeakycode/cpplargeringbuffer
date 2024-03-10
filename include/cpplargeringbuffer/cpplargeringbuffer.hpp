// cpplargeringbuffer - A simple C++ large ring buffer implementation
// Link: https://github.com/squeakycode/cpplargeringbuffer
// Version: 1.0.0
// Minimum required C++ Standard: C++11
// License: BSD 3-Clause License
// 
// Copyright (c) 2024, Andreas Gau
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
\file
\brief Contains a simple C++ large ring buffer implementation
\mainpage
C++ Large Ring Buffer {#pageTitle}
================

##Purpose##
A simple and fast implementation to efficiently store a large amount of
items in a ring buffer with index based access, e.g. for log messages.


##Design Goals##
- Header-only
- KISS principle
- Easy to use and well documented
- Multi-platform
- No build warnings on common compilers
- High test coverage
- Fixed maximum ringbuffer size
- Use buffer segments to manage the memory the ringbuffer needs
- Optimized for index based access to stored objects
- Elements can be inspected using a debugger.

##When to Use##
- Implement a storage for a stream of n objects with index based access
  that are updated continuously but shall not be moved in memory.
*/
#pragma once
#include <vector>
#include <stdexcept>
#include <cassert>

namespace cpplargeringbuffer
{
    /**
        \brief A clear handler that does nothing (default).

        The large ring buffer caches object values. A clear handler can be used
        to free resources when the objects are not used.
    */
    template <typename value_type>
    class noop_clear_handler
    {
    public:
        /**
            \brief Does nothing.
        */
        static void clear(value_type&)
        {
        }
    };

    /**
        \brief A clear handler that replaces unused objects 
              with an instance produced by the default constructor.
        
        The large ring buffer caches object values. A clear handler can be used
        to free resources when the objects are not used.
    */
    template <typename value_type>
    class assign_default_clear_handler
    {
    public:
        /**
            \brief Replace the unused object with a default constructed one.
            \param[in] v    The value to clear.
        */
        static void clear(value_type& v)
        {
            v = value_type();
        }
    };

    /**
        \brief A clear handler that calls a clear() method.

        The large ring buffer caches object values. A clear handler can be used
        to free resources when the objects are not used.
    */
    template <typename value_type>
    class clearable_clear_handler
    {
    public:
        /**
            \brief Calls the clear method of v.
            \param[in] v    The value to clear.
        */
        static void clear(value_type& v)
        {
            v.clear();
        }
    };


    /**
        \brief A ring buffer implementation for a large number of items.

         Implements a storage for a stream of n objects with index based access
         that are updated continuously but shall not be moved in memory.
    */
    template <typename value_type, typename clear_handler_type = noop_clear_handler<value_type> >
    class large_ring_buffer
    {
    public:
        /**
            \brief Constructs a ring buffer object.
        */
        large_ring_buffer() = default;

        /**
            \brief Constructs a ring buffer object and configures it's size parameters.
            \param[in] number_of_segments    The ring buffer is structured in to number_of_segments that are allocated as the ring buffer is filled.
            \param[in] segment_size          The size of a segment in number of items stored.
                                             All items are constructed and destroyed at the same time when needed.

            number_of_segments * segment_size can be used to compute the number of items that can be stored.
        */
        large_ring_buffer(size_t number_of_segments, size_t segment_size)
        {
            discard_and_change_configuration(number_of_segments, segment_size);
        }

        /**
            \brief Destroys a ring buffer object.
        */
        ~large_ring_buffer() = default;

        /**
            \brief Clears a ring buffer object.
            \post
            - The clear method has been called for items stored in the ring buffer.
            - Segment buffers have been cleared.
        */
        void clear()
        {
            const size_t item_count = size();
            for (size_t i = 0; i < item_count; ++i)
            {
                pop_front();
            }

            for (auto& segment : m_segments)
            {
                //completely remove the segment and free the memory
                segment.clear();
                std::vector<value_type> temp;
                segment.swap(temp);
            }
        }

        /**
            \brief Returns the number of items currently stored in the ring buffer.
            \return The number of items currently stored in the ring buffer.
        */
        size_t size() const
        {
            size_t result = 0;
            if (m_end_index == m_start_index)
            {
                if (m_full)
                {
                    result = m_max_size;
                }
                else
                {
                    result = 0;
                }
            }
            else if (m_end_index >= m_start_index)
            {
                result = m_end_index - m_start_index;
            }
            else
            {
                result = (m_max_size - m_start_index) + m_end_index;
            }
            return result;
        }

        /**
            \brief Returns true if no items are currently stored in the ring buffer.
            \return True if no items are currently stored in the ring buffer.
        */
        bool empty() const
        {
            return !m_full && m_end_index == m_start_index;
        }

        /**
            \brief Returns true if the maximum configured number of items are currently stored in the ring buffer.
            \return True if the maximum configured number of items are currently stored in the ring buffer.
        */
        bool full() const
        {
            return m_full;
        }

        /**
            \brief Returns the maximum configured number of items that can be stored in the ring buffer.
            \return The maximum configured number of items that can be stored in the ring buffer.
        */
        size_t get_max_size() const
        {
            return m_max_size;
        }

        /**
            \brief Returns the size of a segment.
            \return The size of a segment.
        */
        size_t get_segment_size() const
        {
            return m_segment_size;
        }

        /**
            \brief Returns the count of segments configured.
            \return The count of segments configured.
        */
        size_t get_segment_count() const
        {
            if (m_max_size && m_segment_size)
            {
                return m_max_size / m_segment_size;
            }
            return 0;
        }

        /**
            \brief Returns the count of segments that are allocated.
            \return The count of segments that are allocated.
        */
        size_t get_used_segments() const
        {
            size_t result = 0;
            for (const auto& segment : m_segments)
            {
                if (!segment.empty())
                {
                    ++result;
                }
            }
            return result;
        }

        /**
            \brief Destroyes all stored objects and configures the size parameters.
            \param[in] number_of_segments    The ring buffer is structured in to number_of_segments that are allocated as the ring buffer is filled.
            \param[in] segment_size          The size of a segment in number of items stored.
                                             All items are constructed and destroyed at the same time when needed.

            number_of_segments * segment_size can be used to compute the number of items that can be stored.
            \post
            - All items stored are destroyed.
            - Clear is not called on the items.
            - The new configuration is applied.
        */
        void discard_and_change_configuration(size_t number_of_segments, size_t segment_size)
        {
            m_segments.clear();
            m_start_index = 0;
            m_end_index = 0;
            m_max_size = 0;
            m_segment_size = 0;
            m_full = false;
            if (number_of_segments == 0 || segment_size == 0)
            {
                //nothing to do here; usable size will be zero
            }
            else
            {
                m_segment_size = segment_size;
                m_segments.resize(number_of_segments);
                m_max_size = number_of_segments * segment_size;
            }
        }

        /**
            \brief Returns a reference to the item stored at the given index.
            \param[in] index    The index of the item in the ring buffer.
            \return The item in the ring buffer at the given index.
        */
        value_type& operator[](size_t index)
        {
            size_t internal_index = to_internal_index(index);
            return get_item(internal_index);
        }

        /**
            \brief Returns a reference to the item stored at the given index.
            \param[in] index    The index of the item in the ring buffer.
            \return The item in the ring buffer at the given index.
        */
        const value_type& operator[](size_t index) const
        {
            size_t internal_index = to_internal_index(index);
            return get_item(internal_index);
        }

        /**
            \brief Returns a reference to the item stored at the given index.
            \param[in] index    The index of the item in the ring buffer.
            \return The item in the ring buffer at the given index.
            Throws an exception if the index is out of bounds.
        */
        value_type& at(size_t index)
        {
            if (index >= size())
            {
                throw std::range_error("Ringbuffer index out of bounds.");
            }
            size_t internal_index = to_internal_index(index);
            return get_item(internal_index);
        }

        /**
            \brief Returns a reference to the item stored at the given index.
            \param[in] index    The index of the item in the ring buffer.
            \return The item in the ring buffer at the given index.
            Throws an exception if the index is out of bounds.
        */
        const value_type& at(size_t index) const
        {
            if (index >= size())
            {
                throw std::range_error("Ringbuffer index out of bounds.");
            }
            size_t internal_index = to_internal_index(index);
            return get_item(internal_index);
        }

        /**
            \brief Adds an item at the back of the ring buffer.
                   Overwrites an item at the front if the ring buffer is full.
            \return The last item in the ring buffer, delivers a cached value or a newly created one.
        */
        value_type& extend_back()
        {
            if (m_full)
            {
                value_type& item = get_item(m_start_index /*start will be overwritten*/);
                clear_handler_type::clear(item);
                increment_start_index();
                increment_end_index();
                return item;
            }
            else
            {
                increment_end_index();
                value_type& item = get_item(before_end_index());
                m_full = (m_start_index == m_end_index);
                return item;
            }
        }

        /**
            \brief Adds an item at the front of the ring buffer.
                   Overwrites an item at the back if the ring buffer is full.
            \return The first item in the ring buffer, delivers a cached value or a newly created one.
        */
        value_type& extend_front()
        {
            if (m_full)
            {
                value_type& item = get_item(before_end_index() /*end will be overwritten*/);
                clear_handler_type::clear(item);
                decrement_start_index();
                decrement_end_index();
                return item;
            }
            else
            {
                decrement_start_index();
                value_type& item = get_item(m_start_index);
                m_full = (m_start_index == m_end_index);
                return item;
            }
        }

        /**
            \brief Removes an item at the back of the ring buffer.
            Results in undefined behavior if the ring buffer is empty() (same as with standard C++ library containers)
        */
        void pop_back()
        {
            assert(!empty());
            clear_handler_type::clear(get_item(before_end_index()));
            decrement_end_index();
            m_full = false;
            if (is_end_at_start_of_segment()) //went to next segment
            {
                remove_unused_segments_back();
            }
        }

        /**
            \brief Removes an item at the front of the ring buffer.
            Results in undefined behavior if the ring buffer is empty() (same as with standard C++ library containers)
        */
        void pop_front()
        {
            assert(!empty());
            clear_handler_type::clear(get_item(m_start_index));
            increment_start_index();
            m_full = false;
            if (is_start_at_start_of_segment()) //went to next segment
            {
                remove_unused_segments_front();
            }
        }

        /**
            \brief Returns the item at the back of the ring buffer.
            \return The item at the back of the ring buffer.
        */
        value_type& back()
        {
            assert(!empty());
            value_type& result = get_item(before_end_index());
            return result;
        }

        /**
            \brief Returns the item at the front of the ring buffer.
            \return The item at the front of the ring buffer.
        */
        value_type& front()
        {
            assert(!empty());
            value_type& result = get_item(m_start_index);
            return result;
        }

        /**
            \brief Returns the item at the back of the ring buffer.
            \return The item at the back of the ring buffer.
        */
        const value_type& back() const
        {
            assert(!empty());
            value_type& result = get_item(before_end_index());
        }

        /**
            \brief Returns the item at the front of the ring buffer.
            \return The item at the front of the ring buffer.
        */
        const value_type& front() const
        {
            assert(!empty());
            value_type& result = get_item(m_start_index);
        }

        /**
            \brief Adds an item at the back of the ring buffer.
                   Overwrites an item at the front if the ring buffer is full.
            \param[in] item     The item to add.
        */
        void push_back(const value_type& item)
        {
            extend_back() = item;
        }

        /**
            \brief Adds an item at the front of the ring buffer.
                   Overwrites an item at the back if the ring buffer is full.
            \param[in] item     The item to add.
        */
        void push_front(const value_type& item)
        {
            extend_front() = item;
        }

    private:
        size_t to_internal_index(size_t index) const
        {
            size_t internal_index = m_start_index + index;
            if (internal_index >= m_max_size)
            {
                internal_index -= m_max_size;
            }
            return internal_index;
        }

        void increment_start_index()
        {
            assert(m_max_size);
            ++m_start_index;
            if (m_start_index == m_max_size)
            {
                m_start_index = 0;
            }
        }

        void decrement_start_index()
        {
            if (m_start_index == 0)
            {
                assert(m_max_size);
                m_start_index = m_max_size - 1;
            }
            else
            {
                --m_start_index;
            }
        }

        void increment_end_index()
        {
            assert(m_max_size);
            ++m_end_index;
            if (m_end_index == m_max_size)
            {
                m_end_index = 0;
            }
        }

        void decrement_end_index()
        {
            if (m_end_index == 0)
            {
                m_end_index = m_max_size - 1;
            }
            else
            {
                --m_end_index;
            }
        }

        size_t before_end_index() const
        {
            size_t result = m_end_index - 1;
            if (m_end_index == 0)
            {
                assert(m_max_size);
                result = m_max_size - 1;
            }
            return result;
        }

        bool is_end_at_start_of_segment() const
        {
            bool result = (m_end_index % m_segment_size == 0);
            return result;
        }

        bool is_start_at_start_of_segment() const
        {
            bool result = (m_start_index % m_segment_size == 0);
            return result;
        }

        void decrement_segment_start(size_t& segment_start, size_t segment_count) const
        {
            if (segment_start == 0)
            {
                segment_start = segment_count - 1;
            }
            else
            {
                --segment_start;
            }
        }

        void increment_segment_end(size_t& segment_end, size_t segment_count) const
        {
            ++segment_end;
            if (segment_end == segment_count)
            {
                segment_end = 0;
            }
        }

        bool can_remove_segments()
        {
            size_t count_unused = m_max_size - size();
            bool result = count_unused > m_segment_size;
            return result;
        }

        void remove_unused_segments_front()
        {
            if (can_remove_segments())
            {
                size_t segment_index = m_start_index / m_segment_size;
                size_t end_segment_index = m_end_index / m_segment_size;
                const size_t segment_count = m_max_size / m_segment_size;

                //keep the segment adjacent to start, to avoid reallocations when index jitters just by one around segment border
                decrement_segment_start(segment_index, segment_count);
                if (segment_index != end_segment_index && !m_segments[segment_index].empty())
                {
                    decrement_segment_start(segment_index, segment_count);
                    auto& segment = m_segments[segment_index];
                    while (segment_index != end_segment_index && !segment.empty())
                    {
                        //completely remove the segment and free the memory
                        segment.clear();
                        std::vector<value_type> temp;
                        segment.swap(temp);
                    }
                }
            }
        }

        void remove_unused_segments_back()
        {
            if (can_remove_segments())
            {
                size_t segment_index = m_end_index / m_segment_size;
                size_t start_segment_index = m_start_index / m_segment_size;
                const size_t segment_count = m_max_size / m_segment_size;

                //keep the segment adjacent to start, to avoid reallocations when index jitters just by one around segment border
                increment_segment_end(segment_index, segment_count);
                if (segment_index != start_segment_index && !m_segments[segment_index].empty())
                {
                    increment_segment_end(segment_index, segment_count);
                    auto& segment = m_segments[segment_index];
                    while (segment_index != start_segment_index && !segment.empty())
                    {
                        //completely remove the segment and free the memory
                        segment.clear();
                        std::vector<value_type> temp;
                        segment.swap(temp);
                    }
                }
            }
        }

        const value_type& get_item(size_t internal_index) const
        {
            size_t segment_index = internal_index / m_segment_size;
            size_t item_index = internal_index % m_segment_size;
            return m_segments[segment_index][item_index];
        }

        value_type& get_item(size_t internal_index)
        {
            size_t segment_index = internal_index / m_segment_size;
            size_t item_index = internal_index % m_segment_size;
            //create segment if needed
            auto& segment = m_segments[segment_index];
            if (segment.empty())
            {
                segment.resize(m_segment_size);
            }
            return m_segments[segment_index][item_index];
        }

        std::vector< std::vector<value_type> > m_segments;
        size_t m_start_index = 0;
        size_t m_end_index = 0;
        bool m_full = false; // if m_start_index == m_end_index indicates either full or empty that's why we need this flag
        size_t m_segment_size = 0;
        size_t m_max_size = 0;
    };
}
