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
#include "WritebackStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (WritebackStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages) {
	W * wreg = (W *) pregs[WREG];
	uint64_t stat = wreg->getstat()->getOutput();
	if(stat != SAOK) {
		return true;
	}
	return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void WritebackStage::doClockHigh(PipeReg ** pregs) {
	RegisterFile * regFile = RegisterFile::getInstance();
	W * wreg = (W *) pregs[WREG];
	uint64_t valE = wreg->getvalE()->getOutput();
	uint64_t dstE = wreg->getdstE()->getOutput();
	uint64_t valM = wreg->getvalM()->getOutput();
	uint64_t dstM = wreg->getdstM()->getOutput();

	bool error = false;
	regFile->writeRegister(valE, dstE, error);
	regFile->writeRegister(valM, dstM, error);
}
