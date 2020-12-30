/**
 * Names: Abdel Issa & Colton Bailey
 * Team: ButWhyThough
 */
#include <iostream>
#include <fstream>
#include <string.h>
#include <ctype.h>

#include "Loader.h"
#include "Memory.h"

//first column in file is assumed to be 0
#define ADDRBEGIN 2   //starting column of 3 digit hex address 
#define ADDREND 4     //ending column of 3 digit hext address
#define DATABEGIN 7   //starting column of data bytes
#define COMMENT 28    //location of the '|' character 


// The last addressed we used
int LAST_ADDRESS = 0x00;



/**
 * Loader constructor
 * Opens the .yo file named in the command line arguments, reads the contents of the file
 * line by line and loads the program into memory.  If no file is given or the file doesn't
 * exist or the file doesn't end with a .yo extension or the .yo file contains errors then
 * loaded is set to false.  Otherwise loaded is set to true.
 *
 * @param argc is the number of command line arguments passed to the main; should
 *        be 2
 * @param argv[0] is the name of the executable
 *        argv[1] is the name of the .yo file
 */
Loader::Loader(int argc, char * argv[]) {
	loaded = false;

	//Start by writing a method that opens the file (checks whether it ends 
	//with a .yo and whether the file successfully opens; if not, return without 
	//loading)

	if(argc >= 2) {
		std::string filename = argv[1];
		if(!fileCheck(argc, filename)) {
			return;
		}
	}else {
		return;
	}

	//The file handle is declared in Loader.h.  You should use that and
	//not declare another one in this file.

	//Next write a simple loop that reads the file line by line and prints it out
	std::string line;
	inf.open(argv[1]);
	int lineNumber = 1;
	while (std::getline(inf, line)) {
			if (hasErrors(line)) {
				std::cout << "Error on line " << std::dec << lineNumber
					<< ": " << line << std::endl;
				return;
			}else {
				std::string specialLetter = line.substr(7,1);
				if(specialLetter.compare(" ")) {
					loadLine(line);
				}
			}
		lineNumber++;
	}

	//Next, add a method that will write the data in the line to memory 
	//(call that from within your loop)

	//Finally, add code to check for errors in the input line.
	//When your code finds an error, you need to print an error message and return.
	//Since your output has to be identical to your instructor's, use this cout to print the
	//error message.  Change the variable names if you use different ones.
	//  std::cout << "Error on line " << std::dec << lineNumber
	//       << ": " << line << std::endl;


	//If control reaches here then no error was found and the program
	//was loaded into memory.
	loaded = true;  

}

/**
 * isLoaded
 * returns the value of the loaded data member; loaded is set by the constructor
 *
 * @return value of loaded (true or false)
 */
bool Loader::isLoaded() {
	return loaded;
}


//You'll need to add more helper methods to this file.  Don't put all of your code in the
//Loader constructor.  When you add a method here, add the prototype to Loader.h in the private
//section.
/* fileCheck
 * checks if file is okay
 *
 * @param: length - length of the filename
 * @param: filename - name of file
 */
bool Loader::fileCheck(int length, std::string filename) {
	if(filename.length() < 4) {
		return false;
	}

	int pos = filename.find(".");

	if(pos == -1) {
		return false;
	}
	
	std::string extension = filename.substr(pos);

	if(extension != ".yo") {
		return false;
	}

	std::ifstream ok;
	ok.open(filename);

	if(!ok.good()) {
		return false;
	}

	return true;

}
/* loadLine
 * loads line
 *
 * @param: line - current line
 */
void Loader::loadLine(std::string line) {
	Memory * mem = Memory::getInstance();
	int i = DATABEGIN;
	int addr = convert(line, ADDRBEGIN, ADDREND);
	
    bool imem_error = false;
	while(line.substr(i) != " " && i < COMMENT-1) {
		if(line.substr(i, 1).find(" ") != std::string::npos) {
			break;
		}
		mem->putByte(convert(line, i, i+1), addr, imem_error);
		
		i += 2;
		addr++;
	}
	LAST_ADDRESS = addr;
}
/* convert
 * converts current line
 *
 * @param: line - current line
 * @param: start - start of line
 * @param: end - end of line
 */
int Loader::convert(std::string line, int start, int end) {
	std::string addr = line.substr(start, end-start+1);
	int answer = std::stoul(addr, nullptr, 16);
	return answer;
}
/* hasErrors
 * checks to see if errors are present
 *
 * @param: line - current line
 */
bool Loader::hasErrors(std::string line) {
	
	bool eol = false;
	bool empty = false;
	int numofbytes = 0;
	for(int i = 0; i < COMMENT + 1; i++) {
		std::string  letter = line.substr(i, 1);
		if(i == 0 && !letter.compare(" ")) {
			empty = true;
			return specialErrors(line);
		}
		if(i == 0 && letter.compare("0")) {
			return true;
		}
		if(i == 1 && letter.compare("x")) {
			return true;
		}
		if( ((i >= 2 && i <= 4) || (i >= 7 && eol == false)) && (letter.compare(" ")) ) {
			if( !isxdigit(letter[0]) ) {
			return true;
			}
			if(i >= 7) {
				numofbytes++;
			}
		}

		if(i == 5 && letter.compare(":")) {
			return true;
		}

		if(i == COMMENT && letter.compare("|")) {
			return true;
		}

		if(eol == true && letter.compare(" ") && i < COMMENT) {
			return true;
		}
		if(i > 7 && !letter.compare(" ")) {
			eol = true;
		}
		if(i == 6 && letter.compare(" ")) {
			return true;
		}
	}
	
	if(empty == false) {
		if(numofbytes % 2 != 0){
			return true;
		}
	}
	
	if(LAST_ADDRESS > convert(line, ADDRBEGIN, ADDREND) || convert(line, ADDRBEGIN, ADDREND)+(numofbytes/2) > 0x1000) {
		return true;
	}
	return false;
}
/* specialErrors
 * checks to see if any special errors are present
 *
 * @param: line - current line
 */
bool Loader::specialErrors(std::string line) {
	for(int i = 0; i < COMMENT + 1; i++) {
		std::string letter = line.substr(i, 1);
		if(i != COMMENT && letter.compare(" ")) {
			return true;
		}
		if(i == COMMENT && letter.compare("|")) {
			return true;
		}
	}
	return false;
}
