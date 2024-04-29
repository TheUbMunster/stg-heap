include_directories(${global-project_SOURCE_DIR}/stg-heap/include)
add_library(stg-heap Heap.cpp HeapStructures.cpp Memory.cpp)