#include "alg/Algorithm.h"
#include <string>
using namespace clang;
using std::string;

int32_t Algorithm::instCounter = 0;

bool Algorithm::execute() {
	return true;
}


StmtPtrSmallVector* Algorithm::ICCopy(Stmt* s) {
	//FIXME memory leak
	StmtPtrSmallVector *res = new StmtPtrSmallVector();
	for(Stmt::child_iterator I = s->child_begin(), E = s->child_end();
			I != E; ++I){
		if(*I) {
			res->push_back(*I);
		}
	}
	return res;
}

NullStmt* Algorithm::AddNewNullStmt() {
	return new (this->resMgr.getCompilerInstance().getASTContext())
		NullStmt(SourceLocation());
}

LabelStmt* Algorithm::AddNewLabel(Stmt *stBody) {
	//FIXME memory leak
	IdentifierInfo &info = getUniqueLabelName();
	DPRINT("info %u: %s %d", (unsigned int)&info, info.getNameStart(), info.getBuiltinID());
	//Sema &S = this->compInst.getSema();
	//LabelDecl *lblD = this->compInst.getSema().LookupOrCreateLabel(&info, stBody->getLocStart());
	//DPRINT("tud: %u\n", (unsigned int)compInst.getASTContext().getTranslationUnitDecl());
	LabelDecl *lblD = LabelDecl::Create(compInst.getASTContext(), 
			compInst.getASTContext().getTranslationUnitDecl(),
			SourceLocation(),
			&info);
	NullStmt *nullSt = new (this->compInst.getASTContext()) NullStmt(SourceLocation());
	return new (this->compInst.getASTContext())
		LabelStmt(SourceLocation(), lblD, 
				stBody ? (Stmt*)stBody : (Stmt*)nullSt);
}

GotoStmt* Algorithm::AddNewGoto(LabelStmt *lblDes) {
	return new (this->resMgr.getCompilerInstance().getASTContext())
		GotoStmt(lblDes->getDecl(), SourceLocation(), SourceLocation());
}

bool Algorithm::renameVarDecl(NamedDecl *D) {
	D->setDeclName(DeclarationName(&getUniqueVarName()));
	return true;
}

bool Algorithm::renameTagDecl(NamedDecl *D) {
	D->setDeclName(DeclarationName(&getUniqueTagName()));
	return true;
}

bool Algorithm::renameNamedDecl(NamedDecl *D) {
	static int counter = 0;
	string lbl("____nameddecl____");
	IdentifierInfo &info = getUniqueIdentifier(lbl, counter);
	D->setDeclName(DeclarationName(&info));
	return true;
}

IdentifierInfo& Algorithm::getUniqueVarName() {
	static int counter = 0;
	const string lbl("____localvar____");
	return getUniqueIdentifier(lbl, counter);
}


IdentifierInfo& Algorithm::getUniqueTagName() {
	static int counter = 0;
	const string lbl("____localtag____");
	return getUniqueIdentifier(lbl, counter);
}

IdentifierInfo& Algorithm::getUniqueLabelName() {
	static int counter = 0;
	const string lbl("____label____");
	return getUniqueIdentifier(lbl, counter);
}

BinaryOperator* Algorithm::BuildAssignExpr(VarDecl *var, Expr* rExpr) {
	DPRINT("--- ASSIGN EXPR BEGIN ---");
	Sema &S = this->resMgr.getCompilerInstance().getSema();
	DeclRefExpr *dExpr = BuildVarDeclRefExpr(var);
	DPRINT("--- DECLREF END ---");
	ExprResult eRes = S.BuildBinOp(0, SourceLocation(), BO_Assign, dExpr, rExpr);
	assert(!eRes.isInvalid());
	DPRINT("--- ASSIGN EXPR END ---");
	return dyn_cast<BinaryOperator>(eRes.get());	
}

BinaryOperator* Algorithm::BuildCommaExpr(Expr *lExpr, Expr *rExpr) {
	DPRINT("Comma BEGIN");
	Sema &S = this->resMgr.getCompilerInstance().getSema();
	ExprResult eRes = S.BuildBinOp(0, SourceLocation(), BO_Comma, lExpr, rExpr);
	assert(!eRes.isInvalid());
	DPRINT("Comma END");
	return dyn_cast<BinaryOperator>(eRes.get());
}

DeclRefExpr* Algorithm::BuildVarDeclRefExpr(VarDecl *var) {
	ValueDecl *D = dyn_cast<ValueDecl>(var);
	assert(D && "cast to ValueDecl failed");
	QualType Ty = var->getType();
	ExprValueKind VK = VK_LValue;
	DeclarationNameInfo NameInfo(D->getDeclName(), SourceLocation());

	DeclRefExpr *e = DeclRefExpr::Create(
			resMgr.getCompilerInstance().getASTContext(),
			NestedNameSpecifierLoc(),
			SourceLocation(), 
			D, false, 
			NameInfo, Ty, VK);
	// Sema::MarkDeclRefReferenced?
	return e;
}

ImplicitCastExpr* Algorithm::BuildImpCastExprToType(Expr *E, QualType Ty, CastKind CK) {
	Sema &S = this->resMgr.getCompilerInstance().getSema();
	ExprResult eRes = S.ImpCastExprToType(E, Ty, CK);
	assert(!eRes.isInvalid());
	return dyn_cast<ImplicitCastExpr>(eRes.get());
}

DeclStmt* Algorithm::BuildVarDeclStmt(VarDecl *VD) {
	return new (resMgr.getCompilerInstance().getASTContext())
		DeclStmt(DeclGroupRef(VD), SourceLocation(), SourceLocation());
}

CXXConstructExpr* Algorithm::BuildTempObjectConstuctExpr(QualType Ty, Expr *expr) {
	assert(expr && "init expr should not be null");
	ASTContext &Ctx = this->resMgr.getCompilerInstance().getASTContext();
	TypeSourceInfo *TInfo = Ctx.CreateTypeSourceInfo(Ty);

	Sema &S = this->resMgr.getCompilerInstance().getSema();
	//FIXME:don't use Sema because it depends on valid SourceLocation
	ExprResult res = S.BuildCXXTypeConstructExpr(TInfo, SourceLocation().getLocWithOffset(1), MultiExprArg(&expr, 1), SourceLocation().getLocWithOffset(1));
	assert(!res.isInvalid());
	return dyn_cast<CXXConstructExpr>(res.get());
}

BinaryOperator* Algorithm::BuildEqualCondExpr(Expr *EL, Expr *ER) {
	Sema &S = this->resMgr.getCompilerInstance().getSema();
	ExprResult eRes = S.BuildBinOp(0, SourceLocation(), clang::BO_EQ, EL, ER);
	assert(!eRes.isInvalid());
	return dyn_cast<BinaryOperator>(eRes.get());
}

BinaryOperator* Algorithm::BuildRangeCondExpr(Expr *EV, Expr *EL, Expr *EH) {
	Sema &S = this->resMgr.getCompilerInstance().getSema();
	ExprResult resLeft = S.BuildBinOp(0, SourceLocation(), clang::BO_LE, EL, EV);
	assert(!resLeft.isInvalid());
	ExprResult resRight = S.BuildBinOp(0, SourceLocation(), clang::BO_LE, EV, EH);
	assert(!resRight.isInvalid());
	ExprResult eRes = S.BuildBinOp(0, SourceLocation(), clang::BO_Add, resLeft.get(), resRight.get());
	assert(!eRes.isInvalid());
	return dyn_cast<BinaryOperator>(eRes.get());
}

IdentifierInfo& Algorithm::getUniqueIdentifier(string sname, int &ccnt) {
	IdentifierTable &idTable = this->compInst.getPreprocessor().getIdentifierTable();
	int csz = idTable.size();
	char lbl[128];
	while(true) {
		sprintf(lbl, "%s%d", sname.c_str(), ccnt++);
		IdentifierInfo& info = idTable.get(string(lbl));
		if(csz < idTable.size()) {
			return info;
		}
	}
}

IntegerLiteral* Algorithm::CrLiteralX(int x) {
	ASTContext &Ctx = resMgr.getCompilerInstance().getSema().getASTContext();
	return new (Ctx)
		IntegerLiteral(Ctx, llvm::APInt(32,x), Ctx.IntTy, SourceLocation());
}

CXXBoolLiteralExpr* Algorithm::BuildCXXBoolLiteralExpr(bool val) {
	ASTContext &Ctx = resMgr.getCompilerInstance().getSema().getASTContext();
	return new (Ctx)
		CXXBoolLiteralExpr(val, Ctx.BoolTy, SourceLocation());
}




//create a new BuiltinType var
DeclStmt* Algorithm::CreateVar(QualType Ty, Expr *initList = NULL, VarDecl::StorageClass SC = clang::SC_Auto) {
	ASTContext &Ctx = resMgr.getCompilerInstance().getSema().getASTContext();
	//add ImpCast if needed
	if(initList) {
		initList = BuildImpCastExprToType(initList, Ty, clang::CK_LValueToRValue);
	}
	VarDecl *VD = VarDecl::Create(Ctx, Ctx.getTranslationUnitDecl(), 
			SourceLocation(), SourceLocation(), 
			&getUniqueVarName(), Ty, NULL, 
			SC, (SC == clang::SC_Auto ? clang::SC_None : SC));
	VD->setInit(initList);

	return BuildVarDeclStmt(VD);
}

//Create a new int var
DeclStmt* Algorithm::CreateIntVar(Expr *initVal = NULL, VarDecl::StorageClass SC = clang::SC_Auto) {
	return CreateVar(resMgr.getCompilerInstance().getSema().getASTContext().IntTy, initVal, SC);
}

//create a new bool var
DeclStmt* Algorithm::CreateBoolVar(Expr *initVal = NULL, VarDecl::StorageClass SC = clang::SC_Auto) {
	return CreateVar(resMgr.getCompilerInstance().getSema().getASTContext().BoolTy, initVal, SC);
}

