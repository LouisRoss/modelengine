IDIR=../../../include
ILDIR=../include
CC=g++
CFLAGS=-I$(IDIR) -I$(ILDIR) -pthread -fpic -std=c++17

ODIR=obj
BDIR=../bin
LDIR=../lib

LIBS= -ldl -ltbb


_DEPS = ModelEngineCommon.h ModelEngineContext.h ModelEngineContextOp.h ModelEngine.h ModelEngineThread.h IModelEnginePartitioner.h AdaptiveWidthPartitioner.h ConstantWidthPartitioner.h IModelEngineWaiter.h ConstantTickWaiter.h FirstWorkWaiter.h WorkerContext.h WorkerContextOp.h Worker.h WorkerThread.h ProcessCallback.h Log.h Recorder.h sdk/ModelRunner.h sdk/IModelInitializer.h sdk/ModelInitializer.h sdk/ModelInitializerProxy.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_LIFEDEPS = LifeCommon.h LifeImplementation.h LifeNode.h LifeOperation.h LifeRecord.h
LIFEDEPS = $(patsubst %,$(ILDIR)/%,$(_LIFEDEPS))

_LOCALDEPS = KeyListener.h
LOCALDEPS = $(patsubst %,./%,$(_LOCALDEPS))

$(ODIR)/%.o: %.cpp $(DEPS) $(LIFEDEPS) $(LOCALDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(BDIR)/ModelLifeInitializer.so $(BDIR)/LifeModel
.PHONY: all

$(BDIR)/ModelLifeInitializer.so: $(ODIR)/ModelLifeInitializer.o
	$(CC) -shared -o $@ $^ $(CFLAGS) $(LIBS)

$(BDIR)/LifeModel: $(ODIR)/LifeModel.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
