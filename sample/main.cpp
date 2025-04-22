//-----------------------------------------------------------------------------
// cpplargeringbuffer
//-----------------------------------------------------------------------------

#include <cpplargeringbuffer/cpplargeringbuffer.hpp>
#include <iostream>
#include <string>

int main()
{
    // create a ring buffer that stores up to 5000000 items
    cpplargeringbuffer::large_ring_buffer<int> ringbuffer(5000000);

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
