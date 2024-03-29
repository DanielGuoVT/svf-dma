//===- DMAPass.h -- Dominator Analysis------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//


/*
 * @file: DMAPass.h
 * @date: 11/12/2017
 * @version: 1.0
 *
 */


#ifndef DMA_H_
#define DMA_H_

#include "MemoryModel/PointerAnalysis.h"
#include "DMA/GlobalBBDomTree.h"
#include "DMA/GlobalBBCFG.h"
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Pass.h>

/*!
 * Whole program pointer analysis.
 * This class performs various pointer analysis on the given module.
 */
// excised ", public llvm::AliasAnalysis" as that has a very light interface
// and I want to see what breaks.
class DMAPass: public llvm::ModulePass { // ModulePass is a class used to implement unstructured interprocedural optimizations and analyses
    typedef std::vector<PointerAnalysis*> PTAVector;

public:
    /// Pass ID
    static char ID;

    enum AliasCheckRule {
        Conservative,	///< return MayAlias if any pta says alias
        Veto,			///< return NoAlias if any pta says no alias
        Precise			///< return alias result by the most precise pta
    };

    /// Constructor needs TargetLibraryInfo to be passed to the AliasAnalysis
    DMAPass() : llvm::ModulePass(ID) {
 
    }

    /// Destructor
    ~DMAPass();

    /// LLVM analysis usage
    virtual inline void getAnalysisUsage(llvm::AnalysisUsage &au) const {
        // declare your dependencies here.
        /// do not intend to change the IR in this pass,
        au.setPreservesAll();
    }

    /// Get adjusted analysis for alias analysis
    virtual inline void* getAdjustedAnalysisPointer(llvm::AnalysisID id) {
        return this;
    }

    /// Interface expose to users of our pointer analysis, given Location infos
    virtual inline llvm::AliasResult alias(const llvm::MemoryLocation  &LocA, const llvm::MemoryLocation  &LocB) {
        return alias(LocA.Ptr, LocB.Ptr);
    }

    /// Interface expose to users of our pointer analysis, given Value infos
    virtual llvm::AliasResult alias(const llvm::Value* V1,	const llvm::Value* V2);

    /// We start from here
    virtual bool runOnModule(llvm::Module& module); // perform the interesting work of the pass

    /// PTA name
    virtual inline llvm::StringRef getPassName() const {
        return "DMAPass";
    }

private:
    /// Create pointer analysis according to specified kind and analyze the module.
    void runPointerAnalysis(llvm::Module& module, u32_t kind);

		void buildGlobalBBDominatorTree();

		void buildGlobalBBCFG(llvm::Module* module);

		void outputDomResult(llvm::Module* module, std::string filepath);

    PTAVector ptaVector;	///< all pointer analysis to be executed.
    PointerAnalysis* _pta;	///<  pointer analysis to be executed.
		GlobalBBCFG* globalBBCFG; // global basic block control flow graph
		llvm::GlobalBBDomTree* globalBBDomTree; // global basick block dominator tree

};


#endif /* DMA_H_ */
