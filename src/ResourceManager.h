#ifndef OBFS_RESOURCEMANAGER_H
#define OBFS_RESOURCEMANAGER_H

#include "stdafx.h"
#include <string>
#include "HeaderSearchPath.h"
#include "Typedef.h"
#include "alg/InitParseConsumer.h"

#include "clang/Rewrite/Rewriter.h"
#include "llvm/Support/Host.h"

using namespace clang;
using std::string;

class ResourceManager {
	OwningPtr<CompilerInstance> compInst;
	OwningPtr<Rewriter> rw;
	
	DeclGroupRefVec decls; 
	
public:
	void init();

	bool initParseAST(string srcMainFile);

	void rewriteToFile(string desMainFile);

	bool prettyPrint(llvm::raw_ostream &out);

	CompilerInstance& getCompilerInstance() {
		return *compInst.get();
	}

	Rewriter& getRewriter() {
		return *rw.get();
	}

	DeclGroupRefVec& getDeclGroupRefVec() {
		return decls;
	}

	//TODO TopLevelDeclGroup is in DeclContext::decls_begin()~decls_end()
	DeclGroupRef& updateAndGetDeclGroupRef();
	DeclGroupRef& getDeclGroupRef();
	
};

#endif
