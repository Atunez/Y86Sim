
//the Fetch stage
class FetchStage: public Stage {
	private:
		void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
				uint64_t rA, uint64_t rB,
				uint64_t valC, uint64_t valP);
		uint64_t selectPC(F * freg, M * mreg, W * wreg);
		bool needRegIds(uint64_t icode);
		bool needValC(uint64_t icode);
		uint64_t predictPC(uint64_t icode, uint64_t valC, uint64_t valP);
		void getRegIds(uint64_t f_pc, uint64_t * rA, uint64_t * rB);
		uint64_t buildValC(uint64_t f_pc, bool RegID);
		uint64_t PCIncrement(uint64_t f_pc, bool need_regids, bool need_valC);
		bool instr_valid(uint64_t icode);
		uint64_t f_stat(bool mem_error, bool instr_valid, uint64_t icode);
		void calcCS(PipeReg ** pregs, Stage ** stages);	
		bool set_D_bubble(uint64_t D_icode, uint64_t M_icode, uint64_t E_icode, uint64_t e_Cnd, uint64_t d_srcA, uint64_t d_srcB, uint64_t E_dstM);
		bool set_D_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB);
		bool set_F_stall(uint64_t D_icode, uint64_t M_icode, uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB);
	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);

};
