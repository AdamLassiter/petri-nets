# List of things to think about...


## Optimisations
* Bottom-substitution optimisation?
    * Early restart at tops of trees for dimension increases?
* Expand matrix on dimension increase rather than copy
    * Does this provide much benefeit? l^(n) vs l^(n+1) or l*(n) vs l*(n+1)
* Store only upper triangular matrix
    * Approx half memory usage
* Store a boolean n-tuple for tokens as (1, 1, 2) ~= (1, 2, 2)
    * Approx one eigth memory usage
* Check converse statement as early halt condition
    * Approx twice time and memory


## Writeup
* Testing and Useful Examples
    * Formal
        * Correctness
        * Consistency
        * Dimensionality
    * Informal
        * What expressions are quick, what are slow
* Differentiate carefully between CL, ALL, MLL
