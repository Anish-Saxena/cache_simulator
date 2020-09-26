### Compile
`make`

### Pre-Execution

Place the `traces` directory in root (`./`).

### Execute
`./driver -trace <trace_name> -parts <number_of_parts> ...`
Check usage by running `./driver` without any arguments.

### Code Layout

The base class is `Functional_Cache` that has simple functions like `Invalidate`, `Fill`, etc. The class creates a functional cache level and maintains it through the functions.

The class implementing the policies is `Cache_Hierarchy` that has a function per policy (Inclusive/ Exclusive/ NINE). The class instantiates 2 objects of `Functional_Cache` class with different `sets` and `ways` to simulate L2 and L3.

The `driver.cpp` file instantiates objects of Cache_Hierarchy as per flags and knobs. By default, multi-threaded simulations (`-mt` flag) are enabled (the simulation itself is sequential) and all 3 policies are simulated. The `-simulate` flag can be used to toggle between various policy combinations.

