#ifndef OBFS_ALG_SIMPLEPRINTERCONSUMER_H
#define OBFS_ALG_SIMPLEPRINTERCONSUMER_H

#include "../stdafx.h"

class SimplePrinterConsumer : public clang::ASTConsumer {
protected:
	llvm::raw_ostream &out;
	clang::CompilerInstance *compInst;

public:
	SimplePrinterConsumer(llvm::raw_ostream &O, clang::CompilerInstance *CI)
		: out(O),
		compInst(CI)
	{}
	~SimplePrinterConsumer(){}

	virtual bool HandleTopLevelDecl(clang::DeclGroupRef D);
};

#endif
