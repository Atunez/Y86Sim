#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "E.h"
#include "W.h"
#include "Stage.h"
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
#include "Memory.h"
#include "Tools.h"
#include "Instructions.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"

bool F_stall;
bool D_stall;
bool D_bubble;

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages) {
	F * freg = (F *) pregs[FREG];
	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];
	D * dreg = (D *) pregs[DREG];

	Memory * mem = Memory::getInstance();


	uint64_t f_pc = 0, icode = 0, ifun = 0, valC = 0, valP = 0;
	uint64_t rA = RNONE, rB = RNONE, stat = SAOK;

	//code missing here to select the value of the PC
	//and fetch the instruction from memory
	//Fetching the instruction will allow the icode, ifun,
	//rA, rB, and valC to be set.
	//The lab assignment describes what methods need to be
	//written.

	bool error = false;
	f_pc = selectPC(freg, mreg, wreg);
	uint64_t instruction = mem->getByte(f_pc, error);

	icode = Tools::getBits(instruction, 4, 7);  
	ifun = Tools::getBits(instruction, 0, 3);

	if(error) {
		icode = INOP;
		ifun = FNONE;
	}

	bool need_regids = needRegIds(icode);
	bool need_valC = needValC(icode);

	if(need_regids) {
		getRegIds(f_pc, &rA, &rB);
	}

	if(need_valC) {
		valC = buildValC(f_pc, need_regids);
	}

	bool instrvalid = instr_valid(icode);
	stat = f_stat(error, instrvalid, icode);

	valP = PCIncrement(f_pc, need_regids, need_valC);

	uint64_t f_predPC = predictPC(icode, valC, valP);

	//The value passed to setInput below will need to be changed
	freg->getpredPC()->setInput(f_predPC);

	calcCS(pregs, stages);

	//provide the input values for the D register
	setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);

	return false;
}
/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs) {
	F * freg = (F *) pregs[FREG];
	D * dreg = (D *) pregs[DREG];

	if(!F_stall) {
		freg->getpredPC()->normal();
	}
	if(D_bubble) {
		dreg->getstat()->bubble(SAOK);
		dreg->geticode()->bubble(INOP);
		dreg->getifun()->bubble();
		dreg->getrA()->bubble(RNONE);
		dreg->getrB()->bubble(RNONE);
		dreg->getvalC()->bubble();
		dreg->getvalP()->bubble();
	}else if(!D_stall) {
		dreg->getstat()->normal();
		dreg->geticode()->normal();
		dreg->getifun()->normal();
		dreg->getrA()->normal();
		dreg->getrB()->normal();
		dreg->getvalC()->normal();
		dreg->getvalP()->normal();
	}
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
 */
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
		uint64_t ifun, uint64_t rA, uint64_t rB,
		uint64_t valC, uint64_t valP) {
	dreg->getstat()->setInput(stat);
	dreg->geticode()->setInput(icode);
	dreg->getifun()->setInput(ifun);
	dreg->getrA()->setInput(rA);
	dreg->getrB()->setInput(rB);
	dreg->getvalC()->setInput(valC);
	dreg->getvalP()->setInput(valP);
}
/* selectPC
 * Program Counter
 *
 * @param: freg - f register
 * @param: mreg - m register
 * @param: wreg - w register
 */
uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg) {
	if(mreg->geticode()->getOutput() == IJXX && (!mreg->getCnd()->getOutput())) {
		return mreg->getvalA()->getOutput();
	}
	if(wreg->geticode()->getOutput() == IRET) {
		return wreg->getvalM()->getOutput();
	}
	return freg->getpredPC()->getOutput();

}
/* needRegIds
 * boolean method to see if register Ids are needed
 *
 * @param: icode - instruction code
 */
bool FetchStage::needRegIds(uint64_t icode) {
	if(icode == IRRMOVQ || icode == IIRMOVQ || icode == IRMMOVQ || icode == IMRMOVQ || icode == IOPQ || icode == IPUSHQ || icode == IPOPQ) {
		return true;
	} 
	return false;
}
/* needValC
 * boolean methos to see if valC is needed
 *
 * @param: icode - instruction code
 */
bool FetchStage::needValC(uint64_t icode) {
	if(icode == IIRMOVQ || icode == IRMMOVQ || icode == IMRMOVQ || icode == IJXX || icode == ICALL) {
		return true;
	} 
	return false;
}
/* predictPC
 * predicts Program Counter
 *
 * @param: icode - instruction code 
 * @param: valC - value of C
 * @param: valP - value of P
 */
uint64_t FetchStage::predictPC(uint64_t icode, uint64_t valC, uint64_t valP) {
	if(icode == IJXX || icode == ICALL) {
		return   valC;
	}
	return   valP;
}
/* getRegIds
 * gets register Ids
 *
 * @param: f_pc - program counter
 * @param: rA - register A input
 * @param: rB - register B input
 */
void FetchStage::getRegIds(uint64_t f_pc, uint64_t * rA, uint64_t * rB) {
	bool error = false;
	Memory * mem = Memory::getInstance();
	*rA = mem->getByte(f_pc+1, error); 
	*rB = mem->getByte(f_pc+1, error);
	*rA = *rA >> 4;
	*rB = *rB & 0x0f;
}
/* buildValC
 * builds value C
 *
 * @param: f_pc - program counter
 * @param: RegID - Id of register
 */
uint64_t FetchStage::buildValC(uint64_t f_pc, bool RegID) {
	bool error = false;
	Memory * mem = Memory::getInstance();
	uint8_t bytes[8];
	if(RegID) {
		for (int i = 0; i < 8; i++) {
			bytes[i] = mem->getByte(f_pc + i + 2, error);
		}
	}else {
		for (int i = 0; i < 8; i++) {
			bytes[i] = mem->getByte(f_pc + i + 1, error);
		}
	}
	return Tools::buildLong(bytes);
}
/* PCIncrement
 * Increment of PC
 *
 * @param: f_pc - program counter
 * @param: need_regids - boolean to see if register Ids are needed
 * @param: need_valcC - boolean to see if valC is needed
 */
uint64_t FetchStage::PCIncrement(uint64_t f_pc, bool need_regids, bool need_valC) {
	if (need_regids && need_valC) {
		return f_pc + 10;
	}else if (need_regids && !need_valC) {
		return f_pc + 2;
	}else if (!need_regids && need_valC) {
		return f_pc + 9;
	}else {
		return f_pc + 1;
	}
}
/* instr_valid
 * Checks to see if current instruction is valid
 *
 * @param: icode - instruction code
 */
bool FetchStage::instr_valid(uint64_t icode) {
	return (icode == INOP || icode == IHALT || icode == IRRMOVQ 
			|| icode == IIRMOVQ || icode == IRMMOVQ || icode == IMRMOVQ 
			|| icode == IOPQ || icode == IJXX || icode == ICALL 
			|| icode == IRET || icode == IPUSHQ || icode == IPOPQ);
}

/* f_stat
 * status of f register
 *
 * @param: mem_error - boolean to represent memory error
 * @param: instr_valid - checks to see if instruction is valid
 * @param: icode - instruction code
 */
uint64_t FetchStage::f_stat(bool mem_error, bool instr_valid, uint64_t icode) {
	if (mem_error) {
		return SADR;
	}else if (!instr_valid) {
		return SINS;
	}else if (icode == IHALT) {
		return SHLT;
	}else {
		return SAOK;
	}
}

/* calcCS
 * calculates conditions
 *
 * @param: pregs - pipeline registers
 * @param: stages - pipeline stages
 */
void FetchStage::calcCS(PipeReg ** pregs, Stage ** stages) {
	DecodeStage * ds = (DecodeStage*) stages[DSTAGE];
	ExecuteStage * es = (ExecuteStage*) stages[ESTAGE];
	E * ereg = (E *) pregs[EREG];
	M * mreg = (M *) pregs[MREG];
	D * dreg = (D *) pregs[DREG];

	uint64_t E_icode = ereg->geticode()->getOutput();
	uint64_t M_icode = mreg->geticode()->getOutput();
	uint64_t D_icode = dreg->geticode()->getOutput();
	uint64_t E_dstM = ereg->getdstM()->getOutput();
	uint64_t d_srcA = ds->getsrcA();
	uint64_t d_srcB = ds->getsrcB();
	uint64_t e_Cnd = es->get_Cnd();

	F_stall = set_F_stall(D_icode, M_icode, E_icode, E_dstM, d_srcA, d_srcB);
	D_stall = set_D_stall(E_icode, E_dstM, d_srcA, d_srcB);
	D_bubble = set_D_bubble(D_icode, M_icode, E_icode, e_Cnd, d_srcA, d_srcB, E_dstM);
}
/* set_F_stall
 * sets stall in register F
 *
 * @param: D_icode - instruction code in D
 * @param: M_icode - instruction code in M
 * @param: E_icode - instruction code in E
 * @param: E_dstM - dest M value in E register
 * @param: d_srcA - source A in d
 * @param: d_srcB - source B in d
 */
bool FetchStage::set_F_stall(uint64_t D_icode, uint64_t M_icode, uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB) {
	return ((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)) || 
		(D_icode == IRET || E_icode == IRET || M_icode == IRET);
}
/* set_D_stall
 * sets stall in register D
 *
 * @param: E_icode - instruction code in E
 * @param: E_dstM - dest M value in E register
 * @param: d_srcA - source A in d
 * @param: d_srcB - source B in d
 */
bool FetchStage::set_D_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB) {
	return (E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB);
}
/* set_D_bubble
 * sets bubble in register D
 *
 * @param: D_icode - insruction code in D
 * @param: M_icode - instruction code in M
 * @param: E_icode - instruction code in E
 * @param: e_Cnd - condition of e
 * @param: d_srcA - source A in d
 * @param: d_srcB - source B in d
 * @param: E_dstM - dest M value in E register
 */
bool FetchStage::set_D_bubble(uint64_t D_icode, uint64_t M_icode, uint64_t E_icode, uint64_t e_Cnd, uint64_t d_srcA, uint64_t d_srcB, uint64_t E_dstM) {
	return (E_icode == IJXX && !e_Cnd) || (!((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)) &&
			(D_icode == IRET || E_icode == IRET || M_icode == IRET));
}


