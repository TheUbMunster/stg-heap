![coverage](https://gitlab.stg.com/stg-portfolio/stg-heap/badges/move-to-makefile/coverage.svg)

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
## Copyright TheUbMunster 2024, all rights reserved.
