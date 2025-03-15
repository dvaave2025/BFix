#include "CFG.h"
#include "CodeObject.h"
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include "InstructionDecoder.h"
#include <iostream>
#include "Operand.h"
#include <time.h>
#include "Instruction.h"
#include "Absloc.h"
#include "AbslocInterface.h"
#include <algorithm>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
#include "BPatch_flowGraph.h"

namespace dp = Dyninst::ParseAPI;
namespace st = Dyninst::SymtabAPI;
using namespace std;
using namespace Dyninst;
using namespace Dyninst::InstructionAPI;

typedef struct odata {
    MachRegister regs[4];
    int mem; 
    bool numsrc;
    long imm; 
    long disp;
    long scale;
    unsigned long uid; 
    bool operator==(const odata& other) const {
        return mem == other.mem && disp == other.disp && scale == other.scale &&
            regs[0] == other.regs[0] && regs[1] == other.regs[1];
    }    
}odata;

