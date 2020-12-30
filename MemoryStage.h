//Class to perform the combinational logic of the Fetch stage;
class MemoryStage: public Stage {
	private:
		void setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE, 
				uint64_t valM, uint64_t dstE, uint64_t dstM);
		bool memRead(uint64_t icode);
		bool memWrite(uint64_t icode);
		uint64_t mem_address(uint64_t icode, uint64_t valA, uint64_t valE);
	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);
		uint64_t get_valM();
		uint64_t get_dstM();
		uint64_t get_mstat();
};
