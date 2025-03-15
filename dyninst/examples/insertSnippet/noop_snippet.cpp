#include <stdio.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
using namespace std;
using namespace Dyninst;
// Create an instance of class BPatch
BPatch bpatch;
// Different ways to perform instrumentation
typedef enum {
	create,
	attach,
	open
}accessType_t;

// Attach, create, or open a file for rewriting
BPatch_addressSpace* startInstrumenting(accessType_t accessType, const char* name, int pid, const char* argv[]) {
	BPatch_addressSpace* handle = NULL;
	switch(accessType) {
		case create:
			handle = bpatch.processCreate(name, argv);
			if (!handle) { fprintf(stderr, "processCreate failed\n"); }
			break;
		case attach:
			handle = bpatch.processAttach(name, pid);
			if (!handle) { fprintf(stderr, "processAttach failed\n"); }
			break;
		case open:
			// Open the binary file and all dependencies
			handle = bpatch.openBinary(name, true);
			if (!handle) { fprintf(stderr, "openBinary failed\n"); }
			break;
	}
	return handle;
}

// Find a point at which to insert instrumentation
std::vector<BPatch_point*>* findPoint(BPatch_addressSpace* app, const char* name, BPatch_procedureLocation loc) {
	std::vector<BPatch_function*> functions;
	std::vector<BPatch_point*>* points;
	// Scan for functions named "name"
	BPatch_image* appImage = app->getImage();
	appImage->findFunction(name, functions);
	if (functions.size() == 0) {
		fprintf(stderr, "No function %s\n", name);
		return points;
	} else if (functions.size() > 1) {
		fprintf(stderr, "More than one %s; using the first one\n", name);
	}
	// Locate the relevant points
	points = functions[0]->findPoint(loc);
	return points;
}

/*bool createAndInsertSnippet1(BPatch_addressSpace* app,std::vector<BPatch_point*>* points) {
	BPatch_image* appImage = app->getImage();
	// Create an increment snippet
	//BPatch_variableExpr* intCounter = app->malloc(*(appImage->findType("unsigned long")), "temp2");
	BPatch_variableExpr* xx = app->malloc(*(appImage->findType("long")), "temp1");
	BPatch_variableExpr* yy = app->malloc(*(appImage->findType("size_t")), "temp2");
	BPatch_arithExpr getv(BPatch_assign, *xx, BPatch_arithExpr
       (BPatch_deref, BPatch_arithExpr(
               BPatch_minus,
               BPatch_registerExpr((MachRegister)0x18010005),
               BPatch_constExpr(0x18))));
	
	if (!app->insertSnippet(getv, *points, BPatch_callBefore, BPatch_firstSnippet)) {
		fprintf(stderr, "insertSnippet failed\n");
		return false;
	}
	return true;
}*/

// Create and insert a free snippet
/*bool createAndInsertSnippet2(BPatch_addressSpace* app, std::vector<BPatch_point*>* points) {
	BPatch_image* appImage = app->getImage();
	// Create the free function call snippet
	std::vector<BPatch_snippet*> freeArgs;
	std::vector<BPatch_snippet*> freeArgs2;
	freeArgs.clear();
	freeArgs2.clear();
	//BPatch_variableExpr* var = NULL;
	//var = appImage->findVariable(*((*points)[0]),"p");
	BPatch_variableExpr* xx = app->malloc(*(appImage->findType("unsigned long")), "temp1");*/
	/*BPatch_arithExpr getv(BPatch_assign, *xx, BPatch_arithExpr
       (BPatch_deref, BPatch_arithExpr(
               BPatch_plus,
               BPatch_registerExpr((MachRegister)0x18010005),
               BPatch_constExpr(-0x18))));*/
	/*BPatch_variableExpr* yy = app->malloc(*(appImage->findType("unsigned long")), "temp2");
	BPatch_arithExpr getv(BPatch_assign, *xx, BPatch_registerExpr((MachRegister)0x1801000d));//r13
    BPatch_arithExpr getv2(BPatch_assign, *yy, BPatch_registerExpr((MachRegister)0x18010005));//r15
	if (!app->insertSnippet(getv, *points, BPatch_callBefore, BPatch_firstSnippet)) {
        fprintf(stderr, "insertSnippet failed\n");
        return false;
    }
	if (!app->insertSnippet(getv2, *points, BPatch_callBefore, BPatch_firstSnippet)) {
		fprintf(stderr, "insertSnippet failed\n");
		return false;
	}
	//var = appImage->findVariable(*((*points)[0]),"temp1");
	if (!xx) {
		printf("Could not find 'temp1' variable\n");
		return false;
	}else{
		freeArgs.push_back(xx);
		freeArgs2.push_back(yy);
	}
	//Find the printf function
	std::vector<BPatch_function*> freeFuncs;
	appImage->findFunction("free", freeFuncs);
	if (freeFuncs.size() == 0) {
		fprintf(stderr, "Could not find free\n");
		return false;
	}
	// Construct a function call snippet
	BPatch_funcCallExpr freeCall(*(freeFuncs[0]), freeArgs);
	// Insert the snippet
	if (!app->insertSnippet(freeCall, *points, BPatch_callBefore, BPatch_lastSnippet)) {
		fprintf(stderr, "insertSnippet failed\n");
		return false;
	}
	BPatch_funcCallExpr freeCall2(*(freeFuncs[0]), freeArgs2);
	if (!app->insertSnippet(freeCall2, *points, BPatch_callBefore, BPatch_lastSnippet)) {
		fprintf(stderr, "insertSnippet failed\n");
		return false;
	}
	return true;
}*/

bool createAndInsertSnippet2(BPatch_addressSpace* app, std::vector<BPatch_point*>* points) {
    BPatch_image* appImage = app->getImage();
    // Create the free function call snippet
    std::vector<BPatch_snippet*> freeArgs;
    //std::vector<BPatch_snippet*> freeArgs2;
    freeArgs.clear();
    //freeArgs2.clear();
    //BPatch_variableExpr* var = NULL;
    //var = appImage->findVariable(*((*points)[0]),"p");
    BPatch_variableExpr* xx = app->malloc(*(appImage->findType("unsigned long")), "temp1");
    /*BPatch_arithExpr getv(BPatch_assign, *xx, BPatch_arithExpr
       (BPatch_deref, BPatch_arithExpr(
               BPatch_plus,
               BPatch_registerExpr((MachRegister)0x18010005),
               BPatch_constExpr(-0x28))));*/
    //BPatch_variableExpr* yy = app->malloc(8, "temp2");
    BPatch_arithExpr getv(BPatch_assign, *xx, BPatch_registerExpr((MachRegister)0x18010002));//rdx
    //BPatch_arithExpr getv2(BPatch_assign, *yy, BPatch_registerExpr((MachRegister)0x18010005));//r15
    if (!app->insertSnippet(getv, *points, BPatch_callBefore, BPatch_firstSnippet)) {
        fprintf(stderr, "insertSnippet failed\n");
        return false;
    }
    //var = appImage->findVariable(*((*points)[0]),"temp1");
    if (!xx) {
        printf("Could not find 'temp1' variable\n");
        return false;
    }else{
        freeArgs.push_back(xx);
        //freeArgs2.push_back(yy);
    }
    //Find the printf function
    std::vector<BPatch_function*> freeFuncs;
    appImage->findFunction("free", freeFuncs);
    if (freeFuncs.size() == 0) {
        fprintf(stderr, "Could not find free\n");
        return false;
    }
    // Construct a function call snippet
    BPatch_funcCallExpr freeCall(*(freeFuncs[0]), freeArgs);
    // Insert the snippet
    if (!app->insertSnippet(freeCall, *points, BPatch_callBefore, BPatch_lastSnippet)) {
        fprintf(stderr, "insertSnippet failed\n");
        return false;
    }
    return true;
}

/*bool createAndInsertSnippet3(BPatch_addressSpace* app, std::vector<BPatch_point*>* points) {
	BPatch_image* appImage = app->getImage();
	std::vector<BPatch_snippet *> snippets;
	snippets.clear();
	std::vector<BPatch_snippet*> freeArgs;
	BPatch_variableExpr* xx;
	//xx = appImage->findVariable(*((*points)[0]),"temp2");
	BPatch_arithExpr getv(BPatch_assign, *xx, appImage->findVariable(*((*points)[0]),"temp2"));
	snippets.push_back(&getv);
	if (!xx) {
        printf("Could not find 'temp2' variable\n");
        return false;
    }else{
        freeArgs.push_back(xx);
        //freeArgs2.push_back(yy);
    }   
    //Find the printf function
    std::vector<BPatch_function*> freeFuncs;
    appImage->findFunction("free", freeFuncs);
    if (freeFuncs.size() == 0) {
        fprintf(stderr, "Could not find free\n");
        return false;
    }   
    // Construct a function call snippet
    BPatch_funcCallExpr freeCall(*(freeFuncs[0]), freeArgs);
	snippets.push_back(&freeCall);
    // Insert the snippet
    if (!app->insertSnippet(snippets, *points, BPatch_callBefore, BPatch_lastSnippet)) {
        fprintf(stderr, "insertSnippet failed\n");
        return false;
    }   
    return true;	
}*/


void finishInstrumenting(BPatch_addressSpace* app, const char* newName)
{
	BPatch_process* appProc = dynamic_cast<BPatch_process*>(app);
	BPatch_binaryEdit* appBin = dynamic_cast<BPatch_binaryEdit*>(app);
	if (appProc) {
		if (!appProc->continueExecution()) {
			fprintf(stderr, "continueExecution failed\n");
		}
		while (!appProc->isTerminated()) {
			bpatch.waitForStatusChange();
		}
	} else if (appBin) {
		if (!appBin->writeFile(newName)) {
			fprintf(stderr, "writeFile failed\n");
		}
	}
}

int main(int argc, const char *argv[]) {
	// Set up information about the program to be instrumented
	const char* progName;
	if (argc != 4) {
		std::cerr << "Usage:\n\t" << argv[0] << " <input binary> <rewrite function> <output binary path>\n";
		return -1; 
	}
	progName = argv[1];
	int progPID = 42;
	const char* progArgv[] = {"InterestingProgram", "-h", NULL};
	accessType_t mode = open;
	// Create/attach/open a binary
	BPatch_addressSpace* app = startInstrumenting(mode, progName, progPID, progArgv);
	if (!app) {
		fprintf(stderr, "startInstrumenting failed\n");
		exit(1);
	}
	// Find the entry point for function InterestingProcedure
	const char* FuncName = argv[2];
	// Find the exit point of main
	std::vector<BPatch_point*> exitPoint;//= findPoint(app, FuncName, BPatch_exit);
	exitPoint.clear();
	app->getImage()->findPoints(0x402517, exitPoint);
	if (/*!exitPoint ||*/ exitPoint.size() == 0) {
		fprintf(stderr, "No exit points for %s\n", FuncName);
		exit(1);
	}
	// Create and insert instrumentation snippet
	/*if (!createAndInsertSnippet1(app, &exitPoint)) {
		fprintf(stderr, "createAndInsertSnippet1 failed\n");
		exit(1);
	}*/
	if (!createAndInsertSnippet2(app, &exitPoint)) {
        fprintf(stderr, "createAndInsertSnippet2 failed\n");
        exit(1);
    }
	/*exitPoint.clear();
	app->getImage()->findPoints(0x4a6aa, exitPoint);
	if (!createAndInsertSnippet3(app, &exitPoint)) {
		fprintf(stderr, "createAndInsertSnippet2 failed\n");
		exit(1);
	}*/

	// Finish instrumentation
	const char* progName2 = argv[3];
	finishInstrumenting(app, progName2);
}

