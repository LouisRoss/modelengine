IDIR=../../include
INITDIR=../../Initializers
ILDIR=../include
IH5DIR=/usr/include/hdf5/serial/
CC=g++
CFLAGS=-I$(IDIR) -I$(INITDIR) -I$(ILDIR) -I$(IH5DIR) -pthread -std=c++17
LOG=-DNOLOG
DEBUGFLAG=

ODIR=obj
LDIR=../../lib

LIBS=-lgtest -lgtest_main -lgmock -ldl -ltbb

_DEPS = ModelEngineCommon.h ModelEngineContext.h ModelEngineContextOp.h ModelEngine.h ModelEngineThread.h IModelEnginePartitioner.h AdaptiveWidthPartitioner.h ConstantWidthPartitioner.h IModelEngineWaiter.h ConstantTickWaiter.h FirstWorkWaiter.h WorkerContext.h WorkerContextOp.h Worker.h WorkerThread.h ProcessCallback.h Log.h Recorder.h sdk/ModelRunner.h sdk/ModelInitializerProxy.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_INITDEPS = IModelInitializer.h ModelInitializer.h 
INITDEPS = $(patsubst %,$(INITDIR)/%,$(_INITDEPS))

_LOCALDEPS = TestImplementation.h TestIdleImplementation.h TestOperation.h TestNode.h TestRecord.h TestHelper.h TestConfigurationRepository.h
LOCALDEPS = $(patsubst %,./%,$(_LOCALDEPS))

_OBJ = ModelEngineTests.o WhenRunningAModel.o WhenPartitioningWork.o WhenDoingSupportFunctions.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS) $(INITDEPS) $(LOCALDEPS)
	$(CC) -c -o $@ $< $(DEBUGFLAG) $(CFLAGS) $(LOG)

all: ModelEngineTests
.PHONY: all

ModelEngineTests: $(OBJ)
	$(CC) -o $@ $^ $(DEBUGFLAG) $(CFLAGS) $(LIBS)

debug:
	make DEBUGFLAG="-g"
.PHONY: debug

.PHONY: log
log:
	make LOG=""

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
