Modified LAMMPS files for thermodynamic integrations. 
The main compute thermo_integ command used in this folder is available at ...

There are four test folders in this folder.
1 - Rerun-Vs-TI-all  ---> comparing the free energy calculated through compute thermo_integ with the energy calculated from the trajectory (rerun).
2 - Rerun-Vs-TI-HAp  ---> ditto for HAp
3 - PeAtom-Vs-TI-all ---> The energy calculated from compute thermo_integ with dlambda=0 is compared with the compute pe/atom output ---> The error is zero.
4 - PeAtom-Vs-TI-HAp ---> ditto for HAp ---> The error is also zero.
