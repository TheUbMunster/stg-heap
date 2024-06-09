![coverage](http://gitlab.stg/stg-portfolio/stg-heap/badges/main/coverage.svg)

## Copyright TheUbMunster 2024, all rights reserved.

# Features
If you're reading this sentence, then this memory allocator still has some issues being worked out & testing that needs to be done that before I would endorse it for "prime time".

* Node coalescing
* Node bifurcation
* Explicit free list (sorted, in-place)
* Implicit page directory (sorted)
* Linear address estimation for searching
* Adjacent page coalescing (TODO)
* Page freeing (with counter thrashing measures) (TODO)
* Fully unit tested (TODO & report code coverage)
* Logarithmic linked list skipping (https://en.wikipedia.org/wiki/Skip_list) (TODO)

# Behavior
* Calling `stg_malloc(size)` will return a 16-byte pointer to a dynamically allocated piece of memory of size greater than or equal to `size`.
* Calling `stg_free(ptr)` will free the memory region created via a previous call to `stg_malloc(size)`. calling `stg_free(ptr)` within the operable lifetime of the program is considered mandatory, and not doing so results in a memory leak.
* Attempting to dereference a ptr previously allocated by `stg_malloc(size)` that was subsequently freed via `stg_free(ptr)` results in undefined behavior.
    - In the circumstance that the memory allocator has freed the page that the ptr previously belonged to: will very likely result in a `segfault`
    - Otherwise, dereferencing and reading the memory may not return what was originally stored there, attempting to modify that memory will likely put the memory allocator in an invalid state, which may cause runtime exceptions.