//class to perform the combinational logic of
//the Fetch stage
class WritebackStage: public Stage {
	private:
	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);

};
