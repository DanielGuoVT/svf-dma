//===- DMAPass.cpp -- Dominator Analysis------------------------------//
//
//                     SVF: Static Value-Flow Analysis
// 
//===-----------------------------------------------------------------------===//

#include "MemoryModel/PointerAnalysis.h"
#include "DMA/DMAPass.h"
#include "WPA/Andersen.h"
#include "WPA/FlowSensitive.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Metadata.h>

using namespace llvm;

char DMAPass::ID = 0;

static RegisterPass<DMAPass> WHOLEPROGRAMPA("dma", "Dominator Analysis");

/// register this into alias analysis group
///static RegisterAnalysisGroup<AliasAnalysis> AA_GROUP(WHOLEPROGRAMPA);

// cl::bits is the class used to represent a list of command line options in the form of a bit vector 
// PTATY is pointer analysis type list
// cl::desc attributes specifies a description for the option to be shown in the -help output for the program
static cl::bits<PointerAnalysis::PTATY> PASelected(cl::desc("Select pointer analysis"),
				cl::values(
						clEnumValN(PointerAnalysis::Andersen_WPA, "nander", "Standard inclusion-based analysis"),
						clEnumValN(PointerAnalysis::AndersenLCD_WPA, "lander", "Lazy cycle detection inclusion-based analysis"),
						clEnumValN(PointerAnalysis::AndersenWave_WPA, "wander", "Wave propagation inclusion-based analysis"),
						clEnumValN(PointerAnalysis::AndersenWaveDiff_WPA, "ander", "Diff wave propagation inclusion-based analysis"),
						clEnumValN(PointerAnalysis::FSSPARSE_WPA, "fspta", "Sparse flow sensitive pointer analysis")
						));


static cl::bits<DMAPass::AliasCheckRule> AliasRule(cl::desc("Select alias check rule"),
				cl::values(
						clEnumValN(DMAPass::Conservative, "conservative", "return MayAlias if any pta says alias"),
						clEnumValN(DMAPass::Veto, "veto", "return NoAlias if any pta says no alias")
						));

static cl::opt<string> OutputFilename("o", cl::desc("Specify path for the output file"), cl::value_desc("filepath"));

/*!
 * Destructor
 */
DMAPass::~DMAPass() {
		PTAVector::const_iterator it = ptaVector.begin();
		PTAVector::const_iterator eit = ptaVector.end();
		for (; it != eit; ++it) {
				PointerAnalysis* pta = *it;
				delete pta;
		}
		ptaVector.clear();
}

/*!
 * We start from here
 */
bool DMAPass::runOnModule(llvm::Module& module)
{
		/// initialization for llvm alias analyzer
		//InitializeAliasAnalysis(this, SymbolTableInfo::getDataLayout(&module));

		for (u32_t i = 0; i< PointerAnalysis::Default_PTA; i++) {  // check which command line parameter has been set 
				if (PASelected.isSet(i))
						runPointerAnalysis(module, i); // run pointer analysis according to parameters 
		}

		//if there is not 
		if(ptaVector.empty()){
				llvm::outs()<<"Error: No Point-to analysis results. Exit Now!\n";
				exit(1);
		}

		_pta->analyze(module);

		// after pointer to analyze, build a global control flow graph in basic block level
		buildGlobalBBCFG(&module);

		// build dominator tree based on the global CFG
		buildGlobalBBDominatorTree();

/* Example code
			print the global control flow graph
		globalBBCFG->dump("basicblock_controlflow_graph.dma");
				print the global dominator tree
		globalBBDomTree->dump("basicblock_dominator_tree.dma");
*/
		outputDomResult(&module, OutputFilename);

		return true;
}

void DMAPass::runPointerAnalysis(llvm::Module& module, u32_t kind)
{
		/// Initialize pointer analysis.
		// _pta is type of PointerAnalysis*
		switch (kind) {
				case PointerAnalysis::Andersen_WPA:
						_pta = new Andersen(); // declared in include/WPA/Andersen.h
						break;
				case PointerAnalysis::AndersenLCD_WPA:
						_pta = new AndersenLCD();
						break;
				case PointerAnalysis::AndersenWave_WPA:
						_pta = new AndersenWave(); // different Andersen algorithm, just more efficient
						break;
				case PointerAnalysis::AndersenWaveDiff_WPA:
						_pta = new AndersenWaveDiff(); // different Andersen algorithm, just more efficient
						break;
				case PointerAnalysis::FSSPARSE_WPA:
						_pta = new FlowSensitive(); 
						break;
				default:
						llvm::outs()<<"Error: No implementation for this point-to analysis.\n";
						return; 
		}
		ptaVector.push_back(_pta); // type of  PTAVector, that is std::vector<PointerAnalysis*>
}


/*!
 * Return alias results based on our points-to/alias analysis
 * TODO: Need to handle PartialAlias and MustAlias here.
 */
llvm::AliasResult DMAPass::alias(const Value* V1, const Value* V2) {

		llvm::AliasResult result = MayAlias;

		PAG* pag = _pta->getPAG();

		/// TODO: When this method is invoked during compiler optimizations, the IR
		///       used for pointer analysis may been changed, so some Values may not
		///       find corresponding PAG node. In this case, we only check alias
		///       between two Values if they both have PAG nodes. Otherwise, MayAlias
		///       will be returned.
		if (pag->hasValueNode(V1) && pag->hasValueNode(V2)) {
				/// Veto is used by default
				if (AliasRule.getBits() == 0 || AliasRule.isSet(Veto)) {
						/// Return NoAlias if any PTA gives NoAlias result
						result = MayAlias;

						for (PTAVector::const_iterator it = ptaVector.begin(), eit = ptaVector.end();
										it != eit; ++it) {
								if ((*it)->alias(V1, V2) == NoAlias)
										result = NoAlias;
						}
				}
				else if (AliasRule.isSet(Conservative)) {
						/// Return MayAlias if any PTA gives MayAlias result
						result = NoAlias;

						for (PTAVector::const_iterator it = ptaVector.begin(), eit = ptaVector.end();
										it != eit; ++it) {
								if ((*it)->alias(V1, V2) == MayAlias)
										result = MayAlias;
						}
				}
		}

		return result;
}

void DMAPass::buildGlobalBBCFG(llvm::Module* module) {
		globalBBCFG = new GlobalBBCFG(module, _pta->getPTACallGraph());
}

void DMAPass::buildGlobalBBDominatorTree() {
		globalBBDomTree = new GlobalBBDomTree(*globalBBCFG);
}


static uint64_t getBBID(llvm::BasicBlock *bb);

//output the results into a destnation file
void DMAPass::outputDomResult(llvm::Module* module, std::string OutputFile){

		std::error_code EC;
		raw_ostream *out = &llvm::outs(); 

		//create a stream output handler
		out	=  new raw_fd_ostream(OutputFile, EC, sys::fs::F_None);

		*out <<"BBID,DOMNUM\n";

		for (Module::iterator F = module->begin(), E = module->end(); F != E; ++F) {

				uint64_t bbcount = 0;
				uint64_t bbid = -1;

				for (Function::iterator B = F->begin(), E = F->end(); B != E; ++B) {

						SmallVector<const BasicBlock*, 8> result;
						globalBBDomTree->getDescendants(&*B, result);

						bbid = getBBID(&*B);
				
						if(bbid == -1)
									continue; 

						*out << bbid << "," << result.size() <<"\n";
						bbcount++; 
				}
		}

		if(out != &llvm::outs())
				delete out; 

}


static uint64_t getBBID(llvm::BasicBlock *bb){

    llvm::MDNode* curLocMDNode = 0;
    for (Instruction& inst: bb->getInstList()){

      if(curLocMDNode = inst.getMetadata("afl_cur_loc"))break;

    }

    if(!curLocMDNode) {
      llvm::outs()<<"getBBID Error 1.\n";
      return -1;
    }

    if(curLocMDNode->getNumOperands() <= 0) {
      
      llvm::outs()<<"getBBID Error 2.\n";
      return -1;
    }

    llvm::Value * val = 	cast<ValueAsMetadata>(curLocMDNode->getOperand(0))->getValue();

    if(!val){
      
      return -1;
      llvm::outs()<<"getBBID Error 3.\n";
    }

    return cast<ConstantInt>(val)->getZExtValue();
}


