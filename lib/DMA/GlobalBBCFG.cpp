#include "DMA/GlobalBBCFG.h"
#include "Util/GraphUtil.h"
#include <llvm/Support/DOTGraphTraits.h>
#include <llvm/IR/BasicBlock.h>

using namespace llvm;

void GlobalBBCFG::buildGlobalBBCFG(llvm::Module* module) {
		//addAllBBCFGNode(module);
		// naive edges means those between basic blocks with preccessor/successor relation
		//addAllNaiveBBCFGEdge(module);
		// advanced edges means those generated due to call/return, thread for/join
		//addAllAdvancedBBCFGEdge(module);
}

void GlobalBBCFG::addAllBBCFGNode(llvm::Module* module) {
//		for (Module::iterator F = module->begin(), E = module->end(); F != E; ++F) {
//				for (Function::iterator B = F->begin(), E = F->end(); B != E; ++B) {
//						addBBCFGNode(&*B); // add all basic blocks as a node 
//				}
//		}
}

void GlobalBBCFG::addAllNaiveBBCFGEdge(llvm::Module* module) {
	//	for (Module::iterator F = module->begin(), E = module->end(); F != E; ++F) {
	//			for (Function::iterator B = F->begin(), E = F->end(); B != E; ++B) {
	//					// fina all edges between a specific bb and its successors -- the naive edge
	//					for (succ_iterator sit = succ_begin(&*B), set = succ_end(&*B); sit != set; ++sit)
	//							addBBCFGEdge(&*B, *sit, GlobalBBCFGEdge::NaiveEdge); 
	//			}
	//	}
}

void GlobalBBCFG::addAllAdvancedBBCFGEdge(llvm::Module* module) {
		// add direct call
	//	for (Module::iterator F = module->begin(), E = module->end(); F != E; ++F) {
	//			PTACallGraphNode* callGraphNode = ptaCallGraph->getCallGraphNode(&*F); // get callGraphNode
	//			// for each out edge of callGraphNode
	//			for (PTACallGraphNode::iterator BE = callGraphNode->OutEdgeBegin(), E = callGraphNode->OutEdgeEnd(); BE != E; ++BE) {
	//					PTACallGraphEdge* callEdge = dynamic_cast<PTACallGraphEdge*>(*BE);
	//					for (auto callinst: callEdge->getDirectCalls()){
	//							const llvm::Function* calleefunc = analysisUtil::getCallee(callinst);
	//							// some funcs do no have entry block, calleefunc->getEntryBlock return NULL
	//							// intrinsic function is a function avaible for use in a given programming 
	//							// language which implementation is handled specially by a compiler
	//							if(!calleefunc || calleefunc->isDeclaration() || calleefunc->isIntrinsic())
	//									continue;
	//							addBBCFGEdge(callinst->getParent(), &calleefunc->getEntryBlock(), GlobalBBCFGEdge::CallRetEdge);
	//					}
	//			}
	//	}

	//	// add indirect call
	//	PTACallGraph::CallEdgeMap indirectCallMap = ptaCallGraph->getIndCallMap();
	//	for (auto calledgemap: indirectCallMap) {
	//			PTACallGraph::FunctionSet calleeFuncSet = calledgemap.second;
	//			for (auto calleefunc: calleeFuncSet) {
	//					if(!calleefunc || calleefunc->isDeclaration() || calleefunc->isIntrinsic())
	//							continue;
	//					addBBCFGEdge(calledgemap.first.getParent(), &calleefunc->getEntryBlock(), GlobalBBCFGEdge::CallRetEdge);
	//			}
	//	}
}

void GlobalBBCFG::addBBCFGNode(const llvm::BasicBlock* bb) {
//		NodeID id = controlFlowGraphNodeNum;
//		GlobalBBCFGNode* controlFlowNode = new GlobalBBCFGNode(id, bb, this);
//		addGNode(id, controlFlowNode);
//		basicBlockToCFGNodeMap[bb] = controlFlowNode;
//		controlFlowGraphNodeNum++;
}

void GlobalBBCFG:: addBBCFGEdge(const llvm::BasicBlock* src, const llvm::BasicBlock* dst, GlobalBBCFGEdge::CEDGEK kind) {
//		GlobalBBCFGNode* srcNode = getCFGNode(src);
//		GlobalBBCFGNode* dstNode = getCFGNode(dst);
//
//		if (!hasGraphEdge(srcNode, dstNode, kind)) {
//				GlobalBBCFGEdge* edge = new GlobalBBCFGEdge(srcNode, dstNode, kind);
//				addEdge(edge);
//		}
}

bool GlobalBBCFG::hasGraphEdge(GlobalBBCFGNode* src, GlobalBBCFGNode* dst, GlobalBBCFGEdge::CEDGEK kind) const {
		GlobalBBCFGEdge edge(src, dst, kind);
		GlobalBBCFGEdge* outEdge = src->hasOutgoingEdge(&edge);
		GlobalBBCFGEdge* inEdge = dst->hasIncomingEdge(&edge);
		if (outEdge && inEdge) {
				assert(outEdge == inEdge && "edges not match");
				return true;
		} else
				return false;
}

//void GlobalBBCFG::dump(const std::string& filename) {
//		GraphPrinter::WriteGraphToFile(llvm::outs(), filename, this);
//}
//
//void GlobalBBCFG::destroy() {
//}

//namespace llvm{
//template<>
//struct DOTGraphTraits<GlobalBBCFG*> : public DefaultDOTGraphTraits {
//		typedef GlobalBBCFGNode NodeType;
//		//typedef NodeType::iterator ChildIteratorType;
//		DOTGraphTraits(bool isSimple = false) :
//				DefaultDOTGraphTraits(isSimple) {
//				}
//
//		static std::string getGraphName(GlobalBBCFG* graph) {
//				return "Global Basic Block Control Flow Graph";
//		}
//
//		static std::string getNodeLabel(GlobalBBCFGNode* node, GlobalBBCFG* graph) {
//				return node->getBasicBlock()->getParent()->getName().str() + "\n" + node->getBasicBlock()->getName().str();
//		}
//
//		static std::string getNodeAttributes(GlobalBBCFGNode* node, GlobalBBCFG* graph) {
//				return "shape=circle";
//		}
//
//		template<class EdgeIter>
//		static std::string getEdgeAttributes(GlobalBBCFGNode* node, EdgeIter EI, GlobalBBCFG* globalBBCFG) {
//				GlobalBBCFGEdge* edge = *(EI.getCurrent());
//				assert(edge && "no edge found");
//				std::string color;
//				if (edge->getEdgeKind() == GlobalBBCFGEdge::NaiveEdge) {
//						color = "color=green";
//				} else if (edge->getEdgeKind() == GlobalBBCFGEdge::CallRetEdge) {
//						color = "color=blue";
//				}
//				return color;
//		}
//
//};
//}

