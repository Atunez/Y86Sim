CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = Memory.o Loader.o RegisterFile.o ConditionCodes.o PipeReg.o Simulate.o Tools.o Simulate.o \
F.o D.o E.o M.o W.o PipeRegField.o ExecuteStage.o MemoryStage.o FetchStage.o DecodeStage.o WritebackStage.o
.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Debug.h Memory.h Loader.h RegisterFile.h ConditionCodes.h PipeReg.h Stage.h Simulate.h Tools.h


Simulate.o: PipeRegField.h Debug.h Memory.h RegisterFile.h ConditionCodes.h \
PipeReg.h Stage.h Simulate.h F.h D.h E.h M.h W.h ExecuteStage.h MemoryStage.h \
DecodeStage.h FetchStage.h WritebackStage.h

ExecuteStage.o: ExecuteStage.h RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h Status.h Debug.h \
ConditionCodes.h Instructions.h MemoryStage.h

MemoryStage.o: MemoryStage.h RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h Status.h Debug.h \
Memory.h Instructions.h

DecodeStage.o: DecodeStage.h RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h Status.h Debug.h \
ExecuteStage.h MemoryStage.h Instructions.h

FetchStage.o: FetchStage.h RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h Status.h Debug.h \
Memory.h Tools.h Instructions.h DecodeStage.h ExecuteStage.h

WritebackStage.o: WritebackStage.h RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h Status.h Debug.h


PipeReg.o: PipeReg.h
PipeRegField.o: PipeRegField.h

F.o: PipeRegField.h PipeReg.h F.h 
D.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h Status.h D.h
E.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h Status.h E.h
M.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h Status.h M.h
W.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h Status.h W.h

Loader.o: Loader.h Memory.h
Memory.o: Memory.h Tools.h
Tools.o: Tools.h
RegisterFile.o: RegisterFile.h Tools.h
ConditionCodes.o: ConditionCodes.h Tools.h

clean:
	rm $(OBJ) yess

run:
	make clean
	make yess
	./run.sh

