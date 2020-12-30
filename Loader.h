//The Loader
class Loader {
	private:
		bool loaded;        //set to true if a file is successfully loaded into memory
		std::ifstream inf;  //input file handle
		bool fileCheck(int length, std::string filename);
		void loadLine(std::string line);
		int convert(std::string line, int start, int end);
		bool hasErrors(std::string line);
		bool specialErrors(std::string line);
	public:
		Loader(int argc, char * argv[]);
		bool isLoaded();
};
