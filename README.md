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

## Getting Started

The easiest way to see the engine in action is to build and run the samples.  The easiest
way to do that is to use the included docker image.

This reduces the dependencies down to:
1. A Linux system capable of running Docker; and
2. Docker.

If you are running Ubuntu, check out installation instruction [here](https://docs.docker.com/engine/install/ubuntu/).

If you are running another Linux distro, look into the installation instructions at [dockerhub](https://hub.docker.com/search?type=edition&offering=community).

Once you have the Docker Engine successfully installed on your system, and have given your user permissions 
to run Docker commands, if necessary, the steps are simple:

1. Create a folder for the source code.  Let's say ~/source.
2. `> cd ~/source`
3. `> git clone https://github.com/LouisRoss/modelengine.git`
4. `> cd modelengine`
5. `> ./dockb`
6. `> ./dock`

At this point, you should have built the docker image and are running it on the same console. 
It will be running as root, in a directory `/home/modelengine`.

You can build and run the samples like this.  The samples use a primitive text-only GUI,
so don't depend on any graphics.

1. `# cd samples/life/source`
2. `# make`
3. `# cd ../bin`
4. `# ./LifeModel life1`

or

1. `# cd samples/particle/source`
2. `# make`
3. `# cd ../bin`
4. `# ./ParticleModel particle1`

In either case, you should build and run one of the samples.  The primitive GUI assumes you have
set your terminal screen to at least 27 rows deep by 50 columns wide.  You should see a tiny
window onto a 1000 X 1000 grid of cells, operating the specified sample.

