#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Memory.h"
#include "Instructions.h"

uint64_t gdstM;
uint64_t gvalM;
uint64_t gstat;

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (MemoryStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages) {
	W * wreg = (W *) pregs[WREG];
	M * mreg = (M *) pregs[MREG];
	Memory * memory = Memory::getInstance();
	uint64_t icode = mreg->geticode()->getOutput(), valE = mreg->getvalE()->getOutput(), valM = 0, valA = mreg->getvalA()->getOutput();
	uint64_t stat =  mreg->getstat()->getOutput(), dstE = mreg->getdstE()->getOutput(), dstM = mreg->getdstM()->getOutput();

	uint64_t address = mem_address(icode, valA, valE);

	bool error = false;
	if(memWrite(icode)) {
		memory->putLong(valA, address, error);
	}
	if(memRead(icode)) {
		valM = memory->getLong(address, error);
	}

	if(error) {
		stat = SADR;
	}

	gdstM = dstM;
	gvalM = valM;
	gstat = stat;

	setWInput(wreg, stat, icode, valE, valM, dstE, dstM);
	return false; 
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs) {
	W * wreg = (W *) pregs[WREG];
	M * mreg = (M *) pregs[MREG];

	wreg->getstat()->normal();
	wreg->geticode()->normal();

	wreg->getvalE()->normal();
	wreg->getvalM()->normal();

	wreg->getdstE()->normal();
	wreg->getdstM()->normal();

	mreg->getstat()->normal();
	mreg->geticode()->normal();
	mreg->getCnd()->normal();
	mreg->getvalA()->normal();
	mreg->getvalE()->normal();
	mreg->getdstE()->normal();
	mreg->getdstM()->normal();

}
/* setWInput 
 * sets W register input
 *
 * @param: wreg - w register
 * @param: stat - status of register
 * @param: icode - instruction code
 * @param: valE - value E
 * @param: valM - value M
 * @param: dstE - dest E value
 * @param: dstM - dest M value
 */
void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE, 
		uint64_t valM, uint64_t dstE, uint64_t dstM) {
	wreg->getstat()->setInput(stat);
	wreg->geticode()->setInput(icode);

	wreg->getvalM()->setInput(valM);
	wreg->getvalE()->setInput(valE);
	wreg->getdstE()->setInput(dstE);
	wreg->getdstM()->setInput(dstM);
}
/* get_valM
 * gets value M
 */
uint64_t MemoryStage::get_valM() {
	return gvalM;
}
/* get_dstM
 * gets dest M
 */
uint64_t MemoryStage::get_dstM() {
	return gdstM;
}
/* get_mstat
 * gets m status
 */
uint64_t MemoryStage::get_mstat() {
	return gstat;
}

/* mem_address
 * gets memory address
 *
 * @param: icode - instruction code
 * @param: valA - value of A
 * @param: valE - value of E
 */
uint64_t MemoryStage::mem_address(uint64_t icode, uint64_t valA, uint64_t valE) {
	if (icode == IRMMOVQ || icode == IPUSHQ 
			|| icode == ICALL || icode == IMRMOVQ) {
		return valE;
	}else if (icode == IPOPQ || icode == IRET) {
		return valA;
	}else {
		return 0;
	}
}
/* memRead
 * boolean to read from memory
 *
 * @param: icode - instruction code
 */
bool MemoryStage::memRead(uint64_t icode) {
	return (icode == IMRMOVQ || icode == IPOPQ || icode == IRET);
}
/* memWrite
 * boolean to write from memory
 *
 * @param: icode - insruction code
 */
bool MemoryStage::memWrite(uint64_t icode) {
	return (icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL);
}
