# Model Engine
## A C++ Engine to efficiently execute Finite-State Automata with all available cores

This project was originally motivated as a platform to efficiently run spiking neural
networks on CPU.  The thing about  *spiking* neural networks (as distinct from the
more static kind used in most deep learning) is that spiking neural networks are sparse.
That is, at any given time, only a very small fraction of the neurons in the network
are actually doing anything.

This sparsity provides an opportunity for runtime efficiencies if you can find a way
to ignore the vast majority of neurons in the network that are not currently processing
information (e.g., spiking now, processing recent inputs from upstream spiking neurons,
in refractory phase, etc.).

In the process of developing something that can efficiently run sparse spiking neural
networks, it became clear that this really should be abstracted to a general purpose
engine to execute any kind of finite-state automata.  This repo is the result of that
abstraction effort.

In order to demonstrate its use, I have included two samples:
- John Conway's game of Life
- A simplified plasma of particles propagating through space.


