#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h"
#include "Stage.h"
#include "DecodeStage.h" 
#include "ExecuteStage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"

uint64_t g_srcA;
uint64_t g_srcB;
uint64_t gEbubble;

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (DecodeStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages) {
	E * ereg = (E *) pregs[EREG];
	D * dreg = (D *) pregs[DREG];

	uint64_t icode = dreg->geticode()->getOutput(), ifun = dreg->getifun()->getOutput(), 
                     valC = dreg->getvalC()->getOutput(), valP = dreg->getvalP()->getOutput(),valA = 0, valB = 0;
	uint64_t stat = dreg->getstat()->getOutput(), rA = dreg->getrA()->getOutput(), 
                    rB = dreg->getrB()->getOutput(),dstE = RNONE, dstM = RNONE, srcA = RNONE, srcB = RNONE;
	bool error = false;
	RegisterFile * regFile = RegisterFile::getInstance();
	srcA = gsrcA(rA, icode);
	srcB = gsrcB(rB, icode);
	dstE = gdstE(rB, icode);
	dstM = gdstM(rA, icode);

	g_srcA = srcA;
	g_srcB = srcB;

	uint64_t E_icode = ereg->geticode()->getOutput();
	uint64_t E_dstM = ereg->getdstM()->getOutput();
	ExecuteStage * es = (ExecuteStage*) stages[ESTAGE];
	uint64_t e_Cnd = es->get_Cnd();

	gEbubble = calcCS(E_icode, E_dstM, srcA, srcB, e_Cnd);

	valA = gselFwdA(srcA, regFile->readRegister(srcA, error), icode, valP, stages, pregs);
	valB = gselFwdB(srcB, regFile->readRegister(srcB, error), stages, pregs);

	setEInput(ereg, stat, icode, ifun, valA, valC, valB, dstE, dstM, srcA, srcB);
	return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs){
	E * ereg = (E *) pregs[EREG];
	if(gEbubble) {
		ereg->getstat()->bubble(SAOK);
		ereg->geticode()->bubble(INOP);
		ereg->getifun()->bubble();
		ereg->getvalC()->bubble();
		ereg->getvalA()->bubble();
		ereg->getvalB()->bubble();
		ereg->getdstE()->bubble(RNONE);
		ereg->getdstM()->bubble(RNONE);
		ereg->getsrcA()->bubble(RNONE);
		ereg->getsrcB()->bubble(RNONE);
	}else {
		ereg->getstat()->normal();
		ereg->geticode()->normal();
		ereg->getifun()->normal();
		ereg->getvalC()->normal();
		ereg->getvalA()->normal();
		ereg->getvalB()->normal();
		ereg->getdstE()->normal();
		ereg->getdstM()->normal();
		ereg->getsrcA()->normal();
		ereg->getsrcB()->normal();

	}

}
/* setEInput
 * sets input of the E register.
 *
 *
 * @param: ereg - E register
 * @param: stat - current status of the program
 * @param: icode - instruction code
 * @param: ifun - instruction function
 * @param: valA - input or register A
 * @param: valB - input or register B
 * @param: valC - input or register C
 * @param: dstE - destination E
 * @param: dstM - destination M
 * @param: srcA - source of A
 * @param: srcB - source of B
 */
void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, 
		uint64_t ifun, uint64_t valA, 
		uint64_t valC, uint64_t valB,
		uint64_t dstE, uint64_t dstM,
		uint64_t srcA, uint64_t srcB) {

	ereg->getstat()->setInput(stat);
	ereg->geticode()->setInput(icode);
	ereg->getifun()->setInput(ifun);
	ereg->getvalA()->setInput(valA);
	ereg->getvalB()->setInput(valB);
	ereg->getvalC()->setInput(valC);
	ereg->getdstE()->setInput(dstE);
	ereg->getdstM()->setInput(dstM);
	ereg->getsrcA()->setInput(srcA);
	ereg->getsrcB()->setInput(srcB);
}
/* gsrcA
 * gets value of srcA
 *
 * @param: rA - input register A
 * @param: icode - instruction code
 */
uint64_t DecodeStage::gsrcA(uint64_t rA, uint64_t icode) {
	if (icode == IRRMOVQ || icode == IRMMOVQ
			|| icode == IOPQ || icode == IPUSHQ) {
		return rA;
	}

	if (icode == IPOPQ || icode == IRET) {
		return RSP;
	}

	else {
		return RNONE;
	}
}
/* gsrcB
 * gets value of srcB
 *
 * @param: rB - input register B
 * @param: icode - instruction code
 */ 
uint64_t DecodeStage::gsrcB(uint64_t rB, uint64_t icode) {
	if (icode == IOPQ || icode == IRMMOVQ
			|| icode == IMRMOVQ) {
		return rB;
	}

	if (icode == IPUSHQ || icode == IPOPQ
			|| icode == ICALL || icode == IRET) {
		return RSP;
	}

	else {
		return RNONE;
	}
}
/* gdstE
 * gets destination E
 *
 * @param: rB - input register B
 * @param: icode - instruction code
 */ 
uint64_t DecodeStage::gdstE(uint64_t rB, uint64_t icode) {
	if (icode == IRRMOVQ || icode == IIRMOVQ
			|| icode == IOPQ) {
		return rB;
	}

	if (icode == IPUSHQ || icode == IPOPQ
			|| icode == ICALL || icode == IRET) {
		return RSP;
	}

	else {
		return RNONE;
	}
}
/* gdstM
 * gets destination M
 *
 * @param: rA - input register A
 * @param: icode - instruction code
 */
uint64_t DecodeStage::gdstM(uint64_t rA, uint64_t icode) {
	if (icode == IMRMOVQ || icode == IPOPQ) {
		return rA;
	}

	else {
		return RNONE;    
	}
}
/* gselFwdA
 * gets Sel + FwdA
 *
 * @param: srcA - value of source A
 * @param: rvalA - value of A
 * @param: icode - instruction code
 * @param: valP - value of P
 * @param: stages - stages of the pipeline
 * @param: pregs - registers of the pipeline
 */ 
uint64_t DecodeStage::gselFwdA(uint64_t srcA, uint64_t rvalA, uint64_t icode, uint64_t valP, Stage ** stages, PipeReg ** pregs) {

	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];
	ExecuteStage * es = (ExecuteStage *) stages[ESTAGE];
	MemoryStage * ms = (MemoryStage *) stages[MSTAGE];

	if (icode == ICALL || icode == IJXX) {
		return valP;
	}
	if(srcA == RNONE){
		return 0;
	}
	if (srcA == es->get_dstE()) {
		return es->get_valE();
	}
	if (srcA == ms->get_dstM()) {
		return ms->get_valM();
	}
	if (srcA == mreg->getdstE()->getOutput()) {
		return mreg->getvalE()->getOutput();
	}
	if (srcA == wreg->getdstM()->getOutput()) {
		return wreg->getvalM()->getOutput();
	}
	if (srcA == wreg->getdstE()->getOutput()) {
		return wreg->getvalE()->getOutput();
	}
	return rvalA;
}
/* gselFwdB
 * gets Sel + FwdB
 *
 * @param: srcB - value of source B
 * @param: rvalB - value of B
 * @param: stages - stages of the pipeline
 *@param: pregs - registers of the pipeline
 */ 
uint64_t DecodeStage::gselFwdB(uint64_t srcB, uint64_t rvalB, Stage ** stages, PipeReg ** pregs) {
	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];
	ExecuteStage * es = (ExecuteStage *) stages[ESTAGE];
	MemoryStage * ms = (MemoryStage *) stages[MSTAGE];

	if (srcB == RNONE) {
		return 0;
	}
	if (srcB == es->get_dstE()) {
		return es->get_valE();
	}
	if (srcB == ms->get_dstM()) {
		return ms->get_valM();
	}
	if (srcB == mreg->getdstE()->getOutput()) {
		return mreg->getvalE()->getOutput();
	}
	if (srcB == wreg->getdstM()->getOutput()) {
		return wreg->getvalM()->getOutput();
	}
	if (srcB == wreg->getdstE()->getOutput()) {
		return wreg->getvalE()->getOutput();
	}
	return rvalB;
}
/* calcCS
 * calculates conditions
 *
 * @param: E_icode - instruction code for E
 * @param: E_dstM - destination M of E register
 * @param: d_srcA - source A of d
 * @param: d_srcB - source B of d
 * @param: e_Cnd - condition of e
 */
bool DecodeStage::calcCS(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB, uint64_t e_Cnd) {
	return (E_icode == IJXX && !e_Cnd) 
		|| ((E_icode == IMRMOVQ 
	    || E_icode == IPOPQ) && (E_dstM == d_srcA 
		|| E_dstM == d_srcB));   
}
/* getsrcA
 * gets source A
 */
uint64_t DecodeStage::getsrcA() {
	return g_srcA;
}
/* getsrcB
 * gets source B
 */ 
uint64_t DecodeStage::getsrcB() {
	return g_srcB;
}




