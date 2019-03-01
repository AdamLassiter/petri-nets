# Petri Nets and Efficient Proof Search using Coalescence
This forms the submission for XX40211 Dissertation, MComp Comp.Sci. w/ Maths. for Adam Lassiter, University of Bath, 2018-19.

## Prerequisites
The following do not represent minimum requirements, but those used in development and testing.  
### LaTeX  
* latexmk 4.59  
* bpextra (from bussproofs)  
* tikz  

### C
* gnu90-compatible cc

## Getting Started
To download the repository:
```
git clone https://gitlab.com/adamlassiter/petri-nets.git
```

To compile and start using the solver:
```
cd petri-nets/src
make clean
make
./sequent (-s) (-t) <expr>
```

To compile the dissertation:
```
cd petri-nets/src
latexmk -pdf dissertation.tex
```

## Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for details on code of conduct and the process for submitting pull requests.

## Authors
This project remains the sole work of Adam Lassiter (https://gitlab.com/adamlassiter, http://people.bath.ac.uk/atl35/) - see [COPYRIGHT.md](COPYRIGHT.md) for details.

## License
This project is licensed under the MIT License - see [LICENSE.md](LICENSE.md) for details.

## Acknowledgements
* Willem Heijltjes - Project Supervisor

