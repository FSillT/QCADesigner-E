Quickstart
==========

The QCADesigner-E is an extension of the QCADesigner (version 2.0.3) [1] and has been developed at the University of Bremen. The tool, which has been presented in [2] (https://doi.org/10.1109/TCAD.2018.2789782), implements the estimation of the power dissipation of QCA circuits based on the works of Timler and Lent et al. [3-5]. The extension is integrated as an additional simulation module that is based on the Coherence Vector Simulation Engine (CVSE). Further, The QCADesigner-E  is fully compatible to QCA designs generated with the QCADesigner version 2.0.3.

QCADesigner-E is free to use and to modify. However, you must add following references in case of any publication:

F. Sill Torres, R. Wille, P. Niemann, and R. Drechsler, “An energy-aware model for the logic synthesis of quantum-dot cellular automata,” IEEE Trans. on CAD of Integrated Circuits and Systems, 2018

K. Walus, T. J. Dysart, G. A. Jullien, and R. A. Budiman, "QCADesigner: a rapid design and Simulation tool for quantum-dot cellular automata," IEEE Transactions on Nanotechnology, vol. 3, pp. 26-31, 2004.

Installation
============
Switch to source folder of the project:

```sh
$ cd QCADesignerE
```

Prepare the program for compiling by typing

```sh
$ ./autogen.sh
```

Next, you have to run the configure script to customize the compilation process for your system. The configure script provides many options. The most commonly used one is --prefix=[directory], where directory is the directory where you would like to install QCADesignerE. Once installed, the QCADesignerE program will be at QCADesignerE/bin/QCADesignerE. For example:

```sh
$ ./configure --prefix=/home/your_account/your_install_location
```

Type ./configure --help for a complete list of configure options.

Compile the program with

```sh
$ make
```
Install the program with

```sh
$ make install
```
Run QCADesignerE with

```sh
$ bin\QCADesigner &
```

Information
===========
- You can find a list of pre-implemented cells in the folder "circuits".

- A detailed description of the QCADesigner can be found in the folder "QCAEnergy\src\docs\manual"

- You can find a description of the parts that have been added to the QCADesigner-E in file "Manual_QDE.pdf"


References
==========
[1]	K. Walus, T. J. Dysart, G. A. Jullien, and R. A. Budiman, "QCADesigner: a rapid design and Simulation tool for quantum-dot cellular automata," IEEE Transactions on Nanotechnology, vol. 3, pp. 26-31, 2004.

[2] F. Sill Torres, R. Wille, P. Niemann, and R. Drechsler, “An energy-aware model for the logic synthesis of quantum-dot cellular automata,” IEEE Trans. on CAD of Integrated Circuits and Systems, accepted, to be published in 2018. 

[3]	J. Timler and C. S. Lent, "Power gain and dissipation in quantum-dot cellular automata," Journal of Applied Physics, vol. 91, pp. 823-831, Jan 15 2002.

[4]	C. S. Lent, L. Mo, and L. Yuhui, "Bennett clocking of quantum-dot cellular automata and the limits to binary logic scaling," Nanotechnology, vol. 17, p. 4240, 2006.

[5]	J. Timler and C. S. Lent, "Maxwell's demon and quantum-dot cellular automata," Journal of Applied Physics, vol. 94, pp. 1050-1060, 2003.
