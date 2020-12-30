//class to perform the combinational logic of
//the Fetch stage
class ExecuteStage: public Stage {
	private:
		void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t valE, 
				uint64_t valA, uint64_t Cnd, uint64_t dstE, uint64_t dstM);
		uint64_t ALUAComp(uint64_t icode, uint64_t valA, uint64_t valC);
		uint64_t ALUBComp(uint64_t icode, uint64_t valB);
		uint64_t ALUFunComp(uint64_t icode, uint64_t ifun);
		bool SetCC(uint64_t icode, uint64_t Mstat, uint64_t Wstat);
		uint64_t dstEComp(uint64_t icode, uint64_t dstE, uint64_t Cnd);
		uint64_t ALUSim(uint64_t ALUvalA, uint64_t ALUvalB, uint64_t ALUfun, bool * of);
		uint64_t cond(uint64_t icode, uint64_t ifun);
		bool calcCS(uint64_t m_stat, uint64_t W_stat);
	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);
		uint64_t get_valE();
		uint64_t get_dstE();
		uint64_t get_Cnd();
};
