### Compile
`make`

### Pre-Execution

Populate the `traces` directory located in base directory (`./`).

### Execute

`./driver -trace <trace_name> -parts <number_of_parts> ...`
Check usage by running `./driver` without any arguments.

### Code Layout

The `Functional_Cache` class has simple functions like `Invalidate`, `Fill`, etc. The class creates a functional cache level and maintains strict LRU order through the functions.

The `Cache_Hierarchy` class implements the cache policies and has a function per policy (Inclusive/ Exclusive/ NINE). The class instantiates 2 objects of `Functional_Cache` class with different `sets` and `ways` to simulate L2 and L3.

The `CacheFullyAssociative` class implements fully-associative LRU and Belady policies at L3 for cache miss classification into cold/ capacity/ conflict. This class is called only in the Inclusive policy case. A progress bar keeps track of Belady simulation, which can take upto a few hours.

The `driver` instantiates objects of `Cache_Hierarchy` class as per flags and knobs. By default, multi-threaded simulations (`-mt` flag) are enabled (each simulation itself is sequential) and all 3 policies are simulated. The `-simulate` flag can be used to toggle between policy combinations.

The `knobs` header file in `include/` directory can be used to toggle L2 and L3 set-associativity, including fully-associative LRU policy for L2 and L3, if required. The `BLK_OFFSET` variable can be used to toggle block size.
