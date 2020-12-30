//class to perform the combinational logic of
//the Fetch stage
class DecodeStage: public Stage {
	private:
		void setEInput(E * ereg, uint64_t stat, uint64_t icode, 
				uint64_t ifun, uint64_t valA, 
				uint64_t valC, uint64_t valB,
				uint64_t dstE, uint64_t dstM,
				uint64_t srcA, uint64_t srcB);
		uint64_t gsrcA(uint64_t rA, uint64_t icode); 
		uint64_t gsrcB(uint64_t rB, uint64_t icode);
		uint64_t gdstE(uint64_t rB, uint64_t icode);
		uint64_t gdstM(uint64_t rA, uint64_t icode);
		uint64_t gselFwdA(uint64_t srcA, uint64_t rvalA, uint64_t icode, uint64_t valP, Stage ** stages, PipeReg ** pregs);
		uint64_t gselFwdB(uint64_t srcB, uint64_t rvalB, Stage ** stages, PipeReg ** pregs);
		bool calcCS(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB, uint64_t e_Cnd);
	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);
		uint64_t getsrcA();
		uint64_t getsrcB();

};
