IDIR=../../../include
INITDIR=../../../include/Initializers
ILDIR=../include
CC=g++
CFLAGS=-I$(IDIR) -I$(INITDIR) -I$(ILDIR) -pthread -fpic -std=c++17
LFLAGS=-fpic -pthread -std=c++17
LOG=-DNOLOG
DEBUGFLAG=

ODIR=obj
BDIR=../bin
LDIR=../lib

LIBS= -ldl -ltbb


_DEPS = ModelEngineCommon.h ModelEngineContext.h ModelEngineContextOp.h ModelEngine.h ModelEngineThread.h IModelEnginePartitioner.h AdaptiveWidthPartitioner.h ConstantWidthPartitioner.h IModelEngineWaiter.h ConstantTickWaiter.h FirstWorkWaiter.h WorkerContext.h WorkerContextOp.h Worker.h WorkerThread.h ProcessCallback.h Log.h Recorder.h sdk/ModelRunner.h sdk/ModelInitializerProxy.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_INITDEPS = IModelInitializer.h ModelInitializer.h ParticleModelInitializer.h 
INITDEPS = $(patsubst %,$(INITDIR)/%,$(_INITDEPS))

_PARTICLEDEPS = ParticleCommon.h ParticleSupport.h ParticleImplementation.h ParticleNode.h ParticleOperation.h ParticleRecord.h
PARTICLEDEPS = $(patsubst %,$(ILDIR)/%,$(_PARTICLEDEPS))

_LOCALDEPS = KeyListener.h
LOCALDEPS = $(patsubst %,./%,$(_LOCALDEPS))

$(ODIR)/%.o: %.cpp $(DEPS) $(INITDEPS) $(PARTICLEDEPS) $(LOCALDEPS)
	$(CC) -c -o $@ $< $(DEBUGFLAG) $(CFLAGS)

all: $(BDIR)/ParticleModelInitializer.so $(BDIR)/ParticleModel
.PHONY: all

log:
	make LOG=""
.PHONY: log

$(BDIR)/ParticleModelInitializer.so: $(ODIR)/ParticleModelInitializer.o
	$(CC) -shared -o $@ $^ $(DEBUGFLAG) $(LFLAGS) $(LIBS)

$(BDIR)/ParticleModel: $(ODIR)/ParticleModel.o
	$(CC) -o $@ $^ $(DEBUGFLAG) $(LFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
