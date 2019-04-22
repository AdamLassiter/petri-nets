# Petri Nets and Efficient Proof Search using Coalescence
This forms the submission for XX40211 Dissertation, MComp Comp.Sci. w/ Maths. for Adam Lassiter, University of Bath, 2018-19.

## Prerequisites
The following do not represent minimum requirements, but those used in development and testing.  
### LaTeX  
* latexmk 4.59  
* bpextra (from bussproofs)  
* tikz  

### C
* gcc 8.2.0

## Getting Started
To download the repository:
```
git clone https://gitlab.com/adamlassiter/petri-nets.git
```

To compile and start using the solver:
```
cd petri-nets/src
make clean
make MAINDEF=SEQUENT_PROOF_MAIN
./proof (-s) (-t) <expr>
```

To compile the dissertation:
```
cd petri-nets/tex
latexmk -pdf dissertation.tex
```

Additionally, the project contains a short abstract submission for  SD’19: 5th Int. Workshop on Structures and Deduction 2019.
To compile the abstract
```
cd petri-nets/tex
latexmk -pdf dortmund.tex
```

## Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for details on code of conduct and the process for submitting pull requests.

## Authors
This project remains the sole work of Adam Lassiter (https://gitlab.com/adamlassiter, http://people.bath.ac.uk/atl35/) and Willem Heijltjes (http://willem.heijltj.es/) - see [COPYRIGHT.md](COPYRIGHT.md) for details.

## License
This project is licensed under the MIT License - see [LICENSE.md](LICENSE.md) for details.

## Acknowledgements
* Willem Heijltjes - Project Supervisor

