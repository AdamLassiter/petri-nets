# From Additive to Classical Proof Search using Coalescence
This forms the submission for XX40211 Dissertation, MComp Comp.Sci. w/ Maths., University of Bath, 2018-19
Furthermore, this forms the submission for Structures and Deduction 2019 (http://www.anupamdas.com/sd19/)

## Prerequisites
The following do not represent minimum requirements, but those used in development and testing.  
### LaTeX  
* latexmk 4.59  
* bpextra (from bussproofs)  
* tikz  

### C
* gcc 8.2.0

## Getting Started
To compile and start using the solver:
```
cd petri-nets/src
make clean
make MAINDEF=SEQUENT_PROOF_MAIN
./proof (-s) (-t) <expr>
```

To compile the dissertation for XX40211:
```
cd petri-nets/tex
latexmk -pdf dissertation.tex
```

Additionally, the project contains a short abstract submission for  SD’19: 5th Int. Workshop on Structures and Deduction 2019.  
To compile the abstract or presentation for SD19:
```
cd petri-nets/sd19
latexmk -pdf abstract.tex
```
or
```
latexmk -pdf presentation.tex
```

## Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for details on code of conduct and the process for submitting pull requests.

## Authors
This project remains the sole work of Adam Lassiter (https://github.com/AdamLassiter) and Willem Heijltjes (http://willem.heijltj.es/) - see [COPYRIGHT.md](COPYRIGHT.md) for details.

## License
This project is licensed under the MIT License - see [LICENSE.md](LICENSE.md) for details.

## Acknowledgements
* Willem Heijltjes - Project Supervisor

