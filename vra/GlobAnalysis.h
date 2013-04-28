/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   GlobAnalysis.h
* @brief  Because for now only intra-procedural analysis is used, we need to
*         recognize if a global variable can be changed in the program or not.
* @date   2013
*/

#ifndef GUARD_GLOB_ANALYSIS_H
#define GUARD_GLOB_ANALYSIS_H

#include <cl/code_listener.h>
#include <cl/storage.hh>

#include <map>
#include <set>
#include <iostream>

#include "ValueAnalysis.h"

/**
* @brief Class recognizes global variables that can be modified by functions and
*        global variables that cannot be modified by functions.
*
*        It is important to know because of usage of intra-procedural analysis.
*        If variable cannot be modified by functions, initializers of the global
*        variable are used. Otherwise, it is over-approximated.
*/
class GlobAnalysis {
	public:
		/// Type used for storing initialization info about global variables.
		typedef std::map<int, bool> GlobVarInit;

		static void computeGlobAnalysis(const CodeStorage::Storage &stor);
		static bool isGlobal(int uid);
		static bool isModified(int uid);
		static std::ostream& printGlobAnalysis(std::ostream &os);

		/// Returns the set of unique ids of all global variables.
		static std::set<int> getGlobVar() { return idOfGlobVarSet; }

		/// Returns the stored ranges for global variables.
		static ValueAnalysis::MemoryPlaceToRangeMap getGlobVarMap()
			{ return globVarMap; }

	private:
		/// Just for assurance that nobody will try to use it.
		GlobAnalysis() { }

		/// Stores the ranges for global variables.
		static ValueAnalysis::MemoryPlaceToRangeMap globVarMap;

		/// Stores the information about all global variables: @c true if variable
		/// was initialized, @c false otherwise.
		static GlobVarInit globVarInit;

		// Stored unique if of all global variables.
		static std::set<int> idOfGlobVarSet;

		static void initGlobVarMap(const CodeStorage::Storage &stor);
		static void storeGlobVar(const CodeStorage::Storage &stor);
		static void initGlobVar();
		static void computeGlobAnalysisForFnc(const CodeStorage::Fnc &fnc);
		static void computeGlobAnalysisForBlock(const CodeStorage::Block *block);
		static void computeGlobAnalysisForInsn(const CodeStorage::Insn *insn);
		static void setIfModified(const CodeStorage::Insn *isns);
		static void processInitial(const CodeStorage::Insn *insn);
		static void processInitialForUnop(const CodeStorage::Insn *insn);
		static void processInitialForBinop(const CodeStorage::Insn *insn);
};

#endif

