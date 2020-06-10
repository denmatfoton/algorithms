# Radix Sort

## Capabilities

`RadixSort` implemented in __sorting.h__ works with arrays of almost all basic C++ types:

- signed/unsigned integers of all possible length (8, 16, 32, 64 bits)
- floating point numbers: float, double (except long double)

_Note: sorting 8 bit numbers should be faster with counting sort._

`RadixSort` can sort in ascending and descending order.

## Performance

### Theory

Radix sort complexity is `O(n * k + m)`, where `n` is array length,
`k = sizeof(array_element) * 8 / radix_bits_num`, i.e. count of digits with radix
`2 ^ radix_bits_num` in the array element, `m = 2 ^ radix_bits_num` (counting array length).
Since `k` and `radix_bits_num` are constants for particular
element type, the resulting complexity is linear `O(n)`.

The best `radix_bits_num` value is 8, i.e. one byte. This requires the counting array
in `RadixSort` to have 256 elements and have total size of 1Kb, which fits entirely
into L1 cache, what is important, because it is accessed randomly during counting step.

### Practice

Testing was performed on i7-7700HQ CPU in Ubuntu Linux.

I performed a series of benchmarks comparing my `RadixSort` with `std::sort`. 
(`std::sort` has known complexity `O(n * log(n))`). I generated random arrays of
different sizes and feed them to both algorithms.

It turned out, that both algorithms have similar performance on arrays with sizes about
100 elements. If array is smaller, `std:sort` performs better because of `m` complexity term.
But if array is significantly larger, `RadixSort` outperforms `std:sort`. For example for
array of length 10'000'000 with elements `int32_t`, `uint32_t` and `float` it is about 16
times faster. You may see it on the graphics below.

![RadixSort vs std::sort (int32_t)](../img/RadixSort_vs_stdsort_(int).png)
![RadixSort vs std::sort (int64_t)](../img/RadixSort_vs_stdsort_(int64_t).png)
![RadixSort vs std::sort (float)](../img/RadixSort_vs_stdsort_(float).png)
![RadixSort vs std::sort (double)](../img/RadixSort_vs_stdsort_(double).png)

## Drawbacks

`RaxixSort` consumes additional memory of linear size during its work, namely
`sizeof(array) + sizeof(counting_array)`. (`counting_array` has constant size). But this
should not be an issue for most applications on modern computers.

In some special cases _counting sort_ or _bucket sort_ may outperform `RadixSort`.

## Possible extensions

### Other array element types

If array element is a complex structure which has a basic numeric field like `int`, and
it is required to sort this array by the numbers in this field, it is possible to make
slight changes in `RadixSort` to perform this task. See example [here](../test/segment_tree_test.cpp).

If array contains strings of small fixed length, it is also possible to modify `RadixSort`
to work well on this task.

### Parallelization

It is possible to parallelize counting step of the radix sort algorithm, but this this will
not bring significant increase of performance, since rearranging step can't be split in subtasks.
It is better to use it in combination with merge sort. Namely perform `RadixSort` in
subtasks on parts of the array and then merge all parts using merge sort.
