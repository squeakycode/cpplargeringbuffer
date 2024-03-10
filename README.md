# C++ Large Ring Buffer

## Purpose
A simple and fast implementation to efficiently store a large amount of
items in a ring buffer with index based access, e.g. for log messages.

## Design Goals
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

## When to Use
- Implement a storage for a stream of n objects with index based access
  that are updated continuously but shall not be moved in memory.

## Sample Code
```
#include <cpplargeringbuffer/cpplargeringbuffer.hpp>
#include <iostream>
#include <string>

int main()
{
    // create a ring buffer that stores up to 5000000 items
    // memory is allocated in batches of 1000 items
    cpplargeringbuffer::large_ring_buffer<int> ringbuffer(5000, 1000);

    // you can add to front and back
    ringbuffer.push_back(1);
    ringbuffer.push_back(2);
    ringbuffer.push_back(3);
    ringbuffer.push_back(4);
    ringbuffer.push_front(0);
    ringbuffer.push_front(123);

    // you can remove from front and back
    ringbuffer.pop_front();
    ringbuffer.pop_back();

    // memory is not moved while adding or removing items

    for (size_t i = 0; i < ringbuffer.size(); ++i)
    {
        //fast index based access
        std::cout << ringbuffer[i] << std::endl;
    }
    //prints:
    //0
    //1
    //2
    //3
}
```
