#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "ConditionCodes.h"
#include "Tools.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h" 
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "MemoryStage.h"

uint64_t gdstE;
uint64_t gvalE;
bool gbubble;
uint64_t gCnd;

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (ExecuteStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages) {
	E * ereg = (E *) pregs[EREG];
	M * mreg = (M *) pregs[MREG];
	uint64_t icode = ereg->geticode()->getOutput(), Cnd = 0, valC = ereg->getvalC()->getOutput(), valA = ereg->getvalA()->getOutput(), valB = ereg->getvalB()->getOutput();
	uint64_t stat = ereg->getstat()->getOutput(), dstE = ereg->getdstE()->getOutput(), dstM = ereg->getdstM()->getOutput(), ifun = ereg->getifun()->getOutput();

	bool of = false;
	uint64_t valE = ALUSim(ALUAComp(icode, valA, valC), ALUBComp(icode, valB), ALUFunComp(icode, ifun), & of);


	bool error = false;
	uint64_t sign = Tools::sign(valE);
	uint64_t zf = 0;
	if(valE == 0) {
		zf = 1;
	}

	MemoryStage * ms = (MemoryStage *) stages[MSTAGE];
	W * wreg = (W *) pregs[WREG];


	uint64_t E_icode = ereg->geticode()->getOutput();
	uint64_t Mstat = ms->get_mstat();
	uint64_t Wstat = wreg->getstat()->getOutput();
	bool changeCC = SetCC(E_icode, Mstat, Wstat);

	if(changeCC) {
		ConditionCodes * cceditor = ConditionCodes::getInstance();
		cceditor->setConditionCode(sign, SF, error);
		cceditor->setConditionCode(zf, ZF, error);
		cceditor->setConditionCode(of, OF, error); 
	}


	gbubble = calcCS(Mstat, Wstat);
	Cnd = cond(icode, ifun);
	dstE = dstEComp(icode, dstE, Cnd);

	gdstE = dstE;
	gvalE = valE;
	gCnd = Cnd;


	setMInput(mreg, stat, icode, valE, valA, Cnd, dstE, dstM);
	return false;

}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg ** pregs) {
	M * mreg = (M *) pregs[MREG];
	if(gbubble) {
		mreg->getstat()->bubble(SAOK);
		mreg->geticode()->bubble(INOP);
		mreg->getCnd()->bubble();
		mreg->getvalE()->bubble();
		mreg->getvalA()->bubble();
		mreg->getdstE()->bubble(RNONE);
		mreg->getdstM()->bubble(RNONE);
	}else {
		mreg->getstat()->normal();
		mreg->geticode()->normal();
		mreg->getCnd()->normal();
		mreg->getvalA()->normal();
		mreg->getvalE()->normal();
		mreg->getdstE()->normal();
		mreg->getdstM()->normal();

	}
}
/* setMInput
 * sets input of M
 *
 * @param: mreg - M register
 * @param: stat - status of pipeline
 * @param: icode - instruction code
 * @param: valE - value E
 * @param: valA - value A
 * @param: Cnd - Condition
 * @param: dstE - destination E
 * @param: dstM - destination M
 */
void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t valE, 
		uint64_t valA, uint64_t Cnd, uint64_t dstE, uint64_t dstM) {
	mreg->getstat()->setInput(stat);
	mreg->geticode()->setInput(icode);

	mreg->getvalA()->setInput(valA);
	mreg->getvalE()->setInput(valE);
	mreg->getCnd()->setInput(Cnd);
	mreg->getdstE()->setInput(dstE);
	mreg->getdstM()->setInput(dstM);
}
/* ALUAComp
 * ALU component A
 *
 * @param: icode - instruction code
 * @param: valA - value of A
 * @param: valC - value of C
 */
uint64_t ExecuteStage::ALUAComp(uint64_t icode, uint64_t valA, uint64_t valC) {
	if (icode == IRRMOVQ || icode == IOPQ) {
		return valA;
	}
	else if (icode == IIRMOVQ || icode == IRMMOVQ || icode == IMRMOVQ) {
		return valC;
	}
	else if (icode == ICALL || icode == IPUSHQ) {
		return -8;
	}
	else if (icode == IRET || icode == IPOPQ) {
		return 8;
	}
	else {
		return 0;
	}
}
/* ALUBComp
 * ALU component B
 *
 * @param: icode - instruction code
 * @param: valB - value of B
 */
uint64_t ExecuteStage::ALUBComp(uint64_t icode, uint64_t valB) {
	if (icode == IRMMOVQ || icode == IMRMOVQ || icode == IOPQ || icode == ICALL
			|| icode == IPUSHQ || icode == IRET || icode == IPOPQ) {
		return valB;
	}
	else if (icode == IRRMOVQ || icode == IIRMOVQ) {
		return 0;
	}
	else {
		return 0;
	}
}
/* ALUFunComp
 * ALU Function Component
 *
 * @param: icode - instruction code
 * @param: ifun - instruction function
 */
uint64_t ExecuteStage::ALUFunComp(uint64_t icode, uint64_t ifun) {
	if (icode == IOPQ) {
		return ifun;
	}
	else {
		return ADDQ;
	}
}
/* SetCC
 * Sets condition codes
 *
 * @param: icode - instruction code
 * @param: Mstat - status of M register
 * @param: Wstat - status of W register
 */
bool ExecuteStage::SetCC(uint64_t icode, uint64_t Mstat, uint64_t Wstat) {
	return (icode == IOPQ) && (Mstat != SADR && Mstat != SINS && Mstat != SHLT)
		&& (Wstat != SADR && Wstat != SINS && Wstat != SHLT);
}
/* dstEComp
 * destination E component
 *
 * @param: icode - instruction code
 * @param: dstE - destination E
 * @param: Cnd - condition
 */
uint64_t ExecuteStage::dstEComp(uint64_t icode, uint64_t dstE, uint64_t Cnd) {
	if (icode == IRRMOVQ && (!Cnd)) {
		return RNONE;
	}
	else {
		return dstE;
	}
}
/* ALUSim
 * ALU simulator
 *
 * @param: ALUvalA - value A of ALU
 * @param: ALUvalB - value B of ALU
 * @param: ALUfun - function of ALU
 * @param: of - overflow flag
 */
uint64_t ExecuteStage::ALUSim(uint64_t ALUvalA, uint64_t ALUvalB, uint64_t ALUfun, bool * of) {
	if(ALUfun == 1) {
		*of = Tools::subOverflow(ALUvalA, ALUvalB);
		return (ALUvalB - ALUvalA);
	}else if(ALUfun == 2) {
		return (ALUvalA & ALUvalB);
	}else if(ALUfun == 3) {
		return (ALUvalA ^ ALUvalB);
	}else {	
		*of = Tools::addOverflow(ALUvalA, ALUvalB);
		return (ALUvalA + ALUvalB);
	}

}
/* get_valE
 * gets value of E
 */
uint64_t ExecuteStage::get_valE() {
	return gvalE;
}
/* get_dstE
 * gets destination value of E
 */
uint64_t ExecuteStage::get_dstE() {
	return gdstE;
}
/* get_Cnd
 * gets condition of Execute Stage
 */
uint64_t ExecuteStage::get_Cnd() {
	return gCnd;
}

/* cond
 * gets condition flags 
 *
 * @param: icode - instruction code
 * @param: ifun - instruction function
 */
uint64_t ExecuteStage::cond(uint64_t icode, uint64_t ifun) {
	bool error = false;
	ConditionCodes * cc = ConditionCodes::getInstance();
	bool sf = cc->getConditionCode(SF, error);
	bool of = cc->getConditionCode(OF, error);
	bool zf = cc->getConditionCode(ZF, error);

	if (icode == IJXX || icode == ICMOVXX) {
		if (ifun == UNCOND){
			return 1;
		}else if (ifun == LESSEQ) {
			return ((sf ^ of) | zf);
		}else if (ifun == LESS) {
			return (sf ^ of);
		}else if (ifun == EQUAL) {
			return (zf);
		}else if (ifun == NOTEQUAL) {
			return (!zf);
		}else if (ifun == GREATER) {
			return (!(sf ^ of) & (!zf));
		}else if (ifun == GREATEREQ) {
			return (!(sf ^ of));
		}
	}

	return 0;
}
/* calcCS
 * calculates conditions
 *
 * @param: Mstat - status of M register
 * @param: Wstat - status of W register
 */
bool ExecuteStage::calcCS(uint64_t Mstat, uint64_t Wstat) {
	return (Mstat == SADR || Mstat == SINS || Mstat == SHLT) ||
		(Wstat == SADR || Wstat == SINS || Wstat == SHLT);
}







