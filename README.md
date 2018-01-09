# Bitfield
This is a single header STL compatable class for bit fields. The usage this code was created for was to be able to create an enum and corresponding array of data related to each enum value. Then optionally set bits in this bitfield depending on features enabled, and later iterate the bitfield and use the data from the array (see example below).

## Example code
```C++
#include <bitfield.h>

enum E
{
    A,
    B,
    C,
    D,
    COUNT
};

int data[COUNT] = {/* The data... */};

bitfield<COUNT> bf;

void set_filter(E v)
{
    bf.set_bit(v);
}

size_t get_data(int* data_out, size_t data_out_len)
{
    size_t c_len = 0;
    for (auto it = bf.begin(); it != bf.end() && c_len < data_out_len; ++it)
    {
        data_out[++c_len] = data[*it];
    }
    
    return c_len;
}
```
## Limitations
The code currently only supports Window x64 platform, but will later be extended to more platforms. 
It also uses some C++11 syntax, but can easily be replaced if needed.

## API description
`bitfield()`
Constructor will memset all data to zeroes. I think the potential bugs that can be mitigate outweight the performance loss in this case. 
Default generated copy constructor and destructor will be used.

`bool check_bit(size_type idx)`
`bool set_bit(size_type idx)`
`bool clear_bit(size_type idx)`
`bool toggle_bit(size_type idx)`
Utility functions to check if bit is set, set a bit, clear a bit, and toggle a bit. All functions return the old value of the bit before changing it. This can be useful to to things such as:
```C++
bitfield<128> bf;
/* ... */
if (bf.clear_bit(DIRTY_BIT))
    do_stuff();
```

`size_type length()`
Returns the number of set bits in the bitfield. Currently uses a hardware specific `popcnt` implementation that may return garbage on old or unsupported hardware.

`iterator begin()`
`const_iterator end() const`
Return STL compatable iterators. Dereferencing iterators returns the index of the bit that is set.
