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

Here is a sample run of the Game of Life sample:

![Life Sample](images/LifeSample.png)

The console updates continuously, so it is good that the model engine keeps one 
hardware core in reserve.  The game board is only initialzed with a few 'living' 
cells, and is not intended to showcase the game of life, only to show that coding
it with the Model Engine is straightforward.

Here is a sample run of the Particle simulator:

![Particle Simulator](images/ParticleSample.png)

In this example, the model is tracking around 13,000 particles of different types, each
tick of the simulation is 50 milliseconds, we have executed 1,016 ticks, processing
5,122,118 individual particle behaviors.  Again, the console updates continuously, 
and any particles within the small window are displayed as they bounce about.

With both samples, there are some single-key commands you can use to steer around
the wider simulation space, control the simuation speed (tick speed in microseconds), 
and quit.  See the legend at the bottom.

### A bit more about the particle simulation

There are actually 15,625 particles in the particle simulation (when it is set to its default
100 X 1000 size), only about 13,000 of which were present in the model at the time of the 
screenshot above.  The rest were in the work queue of the Model Engine, 
waiting to be processed during the next tick.  The way the simulation works is to propagate 
particles by calculating what cell they should propagate to, signaling the Model Engine to 
perform the necessary work the create the particle in that new cell, then erasing the 
particle from its current cell.  During the intervening Model Engine tick, 
the particle is 'in flight', existing only in the Model Engine's work queue.

The screenshots above were taken on a quad-core Intel I7 with hyperthreading, so
a machine with eight available hardware threads.  The Model Engine works in two phases,
first partitioning work among the available threads, then signaling all the worker
threads to consume their partitioned work.  The partitioner guarantees that no two
threads will operate on the same model cell, resulting in lockless operation while
running the model.

Here is a snapshot of the host load while the particle snapshot above (running
in a Docker container) was being taken:

![Host Performance](images/ParticlePerformance.png)

Overall CPU usage was 17.9%, which, given eight effective cores, is equivalent to 104%
of a single core.  If the average work performed per tick is 5,122,118 / 1,016 = 5,041
and the tick speed is 50 milliseconds, or 20 ticks / seconds, then the work item
rate is 5,041 * 20, or around 100,800 work items per second.

The Particle sample is designed to act as a performance load the Model Engine while running
a non-trivial model.  You can easily vary the load by changing the model size in configuration
(see the configuration section below) or using the +/- keys while the model is running
to set the tick speed faster or slower.
