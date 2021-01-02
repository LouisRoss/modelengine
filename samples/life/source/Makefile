IDIR=../../../include
ILDIR=../include
CC=g++
CFLAGS=-I$(IDIR) -I$(ILDIR) -pthread -fpic -std=c++17
LFLAGS=-pthread -fpic -std=c++17
LOG=-DNOLOG

ODIR=obj
BDIR=../bin
LDIR=../lib

LIBS= -ldl -ltbb


_DEPS = ModelEngineCommon.h ModelEngineContext.h ModelEngineContextOp.h ModelEngine.h ModelEngineThread.h IModelEnginePartitioner.h AdaptiveWidthPartitioner.h ConstantWidthPartitioner.h IModelEngineWaiter.h ConstantTickWaiter.h FirstWorkWaiter.h WorkerContext.h WorkerContextOp.h Worker.h WorkerThread.h ProcessCallback.h Log.h Recorder.h sdk/ModelRunner.h sdk/IModelInitializer.h sdk/ModelInitializer.h sdk/ModelInitializerProxy.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_LIFEDEPS = LifeCommon.h LifeSupport.h LifeImplementation.h LifeNode.h LifeOperation.h LifeRecord.h
LIFEDEPS = $(patsubst %,$(ILDIR)/%,$(_LIFEDEPS))

_LOCALDEPS = KeyListener.h
LOCALDEPS = $(patsubst %,./%,$(_LOCALDEPS))

$(ODIR)/%.o: %.cpp $(DEPS) $(LIFEDEPS) $(LOCALDEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LOG)

all: $(BDIR)/ModelLifeInitializer.so $(BDIR)/LifeModel
.PHONY: all

log:
	make LOG=""

.PHONY: log

$(BDIR)/ModelLifeInitializer.so: $(ODIR)/ModelLifeInitializer.o
	$(CC) -shared -o $@ $^ $(LFLAGS) $(LIBS)

$(BDIR)/LifeModel: $(ODIR)/LifeModel.o
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
