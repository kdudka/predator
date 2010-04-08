#ifndef TIMBUK_WRITER_H
#define TIMBUK_WRITER_H

#include <ostream>
#include <string>

class TimbukWriter {

	std::ostream& out;
	
public:

	TimbukWriter(std::ostream& out) : out(out) {}
	
	void startAlphabet() {
		this->out << "Ops";
	}
	
	void writeLabel(size_t label, size_t arity) {
		this->out << " l" << label << ':' << arity;
	}

	void writeLabel(const std::string& label, size_t arity) {
		this->out << ' ' << label << ':' << arity;
	}
	
	void newModel(const std::string& name) {
		this->out << std::endl << "Automaton " << name;
	}
	
	void startStates() {
		this->out << std::endl << "States";
	}
	
	void writeState(size_t state) {
		this->out << " s" << state;
	}

	void writeState(const std::string& state) {
		this->out << ' ' << state;
	}
	
	void startFinalStates() {
		this->out << std::endl << "Final States";
	}
	
	void startTransitions() {
		this->out << std::endl << "Transitions";
	}
	
	void writeTransition(const std::vector<size_t>& lhs, size_t label, size_t rhs) {
		this->out << std::endl << 'l' << label << '(';
		if (lhs.size() > 0) {
			this->out << 's' << lhs[0];
			for (size_t i = 1; i < lhs.size(); ++i)
				this->out << ",s" << lhs[i];
		}
		this->out << ")->s" << rhs;
	}

	void writeTransition(const std::vector<string>& lhs, const std::string& label, const std::string& rhs) {
		this->out << std::endl << label << '(';
		if (lhs.size() > 0) {
			this->out << lhs[0];
			for (size_t i = 1; i < lhs.size(); ++i)
				this->out << ',' << lhs[i];
		}
		this->out << ")->" << rhs;
	}
	
	void terminate() {
		this->out << std::endl;
	}

};

#endif
