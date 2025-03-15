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

#define DB 0 

namespace dp = Dyninst::ParseAPI;
namespace st = Dyninst::SymtabAPI;
using namespace std;
using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
unsigned long uid = 1;

struct odata {
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
};

typedef struct odata odata;

struct PairHash {
    std::size_t operator()(const std::pair<dp::Block*, Dyninst::Address>& p) const {
        return std::hash<dp::Block*>{}(p.first) ^ std::hash<Dyninst::Address>{}(p.second);
    }
};

// Custom equality function for pairs
struct PairEqual {
    bool operator()(const std::pair<dp::Block*, Dyninst::Address>& p1, const std::pair<dp::Block*, Dyninst::Address>& p2) const {
        return p1.first == p2.first && p1.second == p2.second;
    }
};

dp::CodeObject::funclist all;
//std::map<dp::Block *, int> blabel;
std::map<dp::Block *, std::pair <int, unsigned long>> blabel;
std::vector<odata> aset;
std::vector<MachRegister> saved = {(MachRegister)0x18010004, (MachRegister)0x18010005, (MachRegister)0x1801000e};//rsp, rbp
std::vector<MachRegister> setimul = {(MachRegister)0x18010000, (MachRegister)0x18010002};//rax, rdx
std::vector<std::vector<odata>> vova;
std::vector<std::vector<odata>> vovi;
std::map<unsigned long, std::pair <dp::Block *, Dyninst::Address>> eachpath;
std::vector<dp::Block *> ubs;
bool notpair = false;
unsigned long pathnum = 0;
dp::Block *lastuseb = NULL;
Dyninst::Address lastusepc = 0x0;
bool fiteachpath = false;
std::map<dp::Block *, std::vector <odata>>infomap;

MachRegister unif(MachRegister reg){
	MachRegister uni;
	switch(reg){
		case 0x18010000: case 0x18010500://rax
			uni = (MachRegister)0x18010000;
			break;
		case 0x18010001: case 0x18010501://rcx
			uni = (MachRegister)0x18010001;
			break;
		case 0x18010002: case 0x18010502://rdx
            uni = (MachRegister)0x18010002;
            break;
		case 0x18010003: case 0x18010503://
            uni = (MachRegister)0x18010003;
            break;
		case 0x18010004: case 0x18010504://rsp
            uni = (MachRegister)0x18010004;
            break;
		case 0x18010005: case 0x18010505://rbp
            uni = (MachRegister)0x18010005;
            break;
		case 0x18010006: case 0x18010506://rsi
            uni = (MachRegister)0x18010006;
            break;
		case 0x18010007: case 0x18010507://rdi
            uni = (MachRegister)0x18010007;
            break;
		case 0x18010008: case 0x18010508://
            uni = (MachRegister)0x18010003;
            break;
		case 0x18010009: case 0x18010509://
            uni = (MachRegister)0x18010009;
            break;
		case 0x1801000a: case 0x1801050a://
            uni = (MachRegister)0x1801000a;
            break;
		case 0x1801000b: case 0x1801050b://
            uni = (MachRegister)0x1801000b;
            break;
		case 0x1801000c: case 0x1801050c://
            uni = (MachRegister)0x1801000c;
            break;
		case 0x1801000d: case 0x1801050d://
            uni = (MachRegister)0x1801000d;
            break;
		case 0x1801000e: case 0x1801050e://
            uni = (MachRegister)0x1801000e;
            break;
		case 0x1801000f: case 0x1801050f://
            uni = (MachRegister)0x1801000f;
            break;
		default:
			uni = reg;
	}
	return uni;
}

//malloc: 1; calloc :2; realloc: 3; free: 4; Not found: 0;
int calltype(dp::CodeObject::funclist fs, Dyninst::Address target){
	for (auto const *ff : fs) {
		//cout <<hex <<ff->addr() << "---" << target << "\n";
		if(ff->addr() == target){
			if(ff->name() == "malloc"){
				return 1;
			}else if(ff->name() == "calloc"){
				return 2;
			}else if(ff->name() == "realloc"){
				return 3;
			}else if(ff->name() == "xalloc"){
				return 4;
			}else if(ff->name() == "memset"){
				return 0xa;
			}else if(ff->name() == "free"){
				return 0xffff;
			}else{
				return 0;
			}
		}
	}//5 belongs to ret
	return -1;
}

void managevova(odata dst, odata src, bool changeT){
	std::vector<odata> atemp;
    atemp.clear();
	bool fda = false;
	if(vova.size() == 0){
		if(!src.numsrc){
			atemp.push_back(src);
			atemp.push_back(dst);
			vova.push_back(atemp);
		}
	}else{
		for (size_t k = 0; k < vova.size(); ++k) {
            auto x1 = std::find(vova[k].begin(), vova[k].end(), dst);
			if(x1 != vova[k].end() && changeT) {
                vova[k].erase(x1);
            }
        }
		for (size_t k = 0; k < vova.size(); ++k) {
			auto x2 = std::find(vova[k].begin(), vova[k].end(), src);
			if(x2 != vova[k].end() && changeT && !src.numsrc){
                dst.uid = (*x2).uid;
				vova[k].push_back(dst);
                fda = true;
            }
		}
		if(!fda){
			if(!src.numsrc && changeT){
				atemp.push_back(src);
				atemp.push_back(dst);
				vova.push_back(atemp);
			}
		}
		
	}
	if(vovi.size() != 0){
		for (size_t i = 0; i < vovi.size(); ++i) {
            auto x3 = std::find(vovi[i].begin(), vovi[i].end(), dst);
            auto x4 = std::find(vovi[i].begin(), vovi[i].end(), src);
			if (x3 != vovi[i].end()) {
				vovi[i].erase(x3);
            }
			if (x4 != vovi[i].end()) {
				vovi[i].push_back(dst);
			}
        }
    }
}

void managevovi(odata dst, odata src){
    std::vector<odata> itemp;
    itemp.clear();
	bool fdi = false;
    if(vovi.size() == 0){
		if(!src.numsrc){
			itemp.push_back(src);
			itemp.push_back(dst);
			vovi.push_back(itemp);
		}
    }else{
		for (size_t k = 0; k < vovi.size(); ++k) {
            auto x1 = std::find(vovi[k].begin(), vovi[k].end(), dst);
            auto x2 = std::find(vovi[k].begin(), vovi[k].end(), src);
            if(x1 != vovi[k].end()) {
				vovi[k].erase(x1);
            }
        }
		for (size_t k = 0; k < vovi.size(); ++k) {
            auto x2 = std::find(vovi[k].begin(), vovi[k].end(), src);
            if(x2 != vovi[k].end()){
                dst.uid = (*x2).uid;
				vovi[k].push_back(dst);
                fdi = true;
            }   
        }
        if(!fdi){
			if(!src.numsrc){
				itemp.push_back(src);
				itemp.push_back(dst);
				vovi.push_back(itemp);
			}
        }
    }
    if(vova.size() != 0){
		for (size_t k = 0; k < vova.size(); ++k){
            auto x3 = std::find(vova[k].begin(), vova[k].end(), dst);
			if (x3 != vova[k].end()) {
				vova[k].erase(x3);
			}
		}
    }
}

std::vector<odata> intersection(const std::vector<odata>& v1, const std::vector<odata>& v2) {
    std::vector<odata> result;
    for (odata num : v1) {
        if (std::find(v2.begin(), v2.end(), num) != v2.end()) {
            result.push_back(num);
        }
    }
    return result;
}

void full_aliasanalysis(dp::Function const *f,dp::Block *b, odata data, bool memr, bool memw, 
unsigned long opc, int size, Dyninst::Address firstAddr, dp::Block *xxb, bool findp){
#if 1
    cout << size << "\n";
    /*for(int i = 0; i < 4; i++){
        if(data.regs[i] != 0)
            printf("%d, %lx ,%s\n", i, data.regs[i], data.regs[i].name().c_str());
    }
    printf("memr:%d, memw:%d, mem:%d, numsrc:%d, imm:%lx, disp:%lx, scale:%lx\n",
        memr, memw, data.mem, data.numsrc, data.imm, data.disp, data.scale);*/
#endif
	if(findp){
		if(firstAddr == xxb->start()){
			std::vector<odata> ted;
			ted.clear();
			for (size_t k = 0; k < vova.size(); ++k) {
				for(odata xxx : vova[k]){
					if(xxx.uid > 0){
						cout << "FIND-:::";
						cout << xxx.regs[0] << "-"<< xxx.regs[1]<< "--"<< xxb->start()<<"\n";
						auto xt = std::find(ted.begin(), ted.end(), xxx);
						if(xt == ted.end()){
							ted.push_back(xxx);
						}
						fiteachpath = true;
						break;
					}
				}
			}
			if(infomap[xxb].size() != 0){
				std::vector<odata> ted2;
				ted2.clear();
				ted2 = infomap[xxb];
				infomap[xxb].clear();
				infomap[xxb] = intersection(ted2, ted);
			}else{
				infomap[xxb] = ted;
			}
		}
	}
    odata oper[4];
	memset(oper, 0, sizeof(oper));
	unsigned long tttt = 0;
    if(size == 3){

    }else if(size == 2){
        if(data.numsrc){
            if(memw){
                memcpy(oper[0].regs, data.regs, sizeof(data.regs));
                oper[0].mem = 1;
                oper[0].disp = data.disp;
                oper[0].scale = data.scale;
                oper[1].imm = data.imm;
                oper[1].numsrc = true;
            }else{
                memcpy(oper[0].regs, data.regs, sizeof(data.regs));
                oper[0].mem = 0;
                oper[1].imm = data.imm;
                oper[1].numsrc = true;
            }
        }else{
            if(memr){
                oper[0].regs[0] = data.regs[0];
                for(int i = 1; i < 4; i++){
                    if(data.regs[i] != 0)
                        oper[1].regs[i-1] = data.regs[i];
                }
                oper[1].mem = 1;
                oper[1].disp = data.disp;
                oper[1].scale = data.scale;
            }else if(memw){
                int k = 0;
                for(int i = 0; i < 4; i++){
                    if(data.regs[i+1] != 0){
                        oper[0].regs[i] = data.regs[i];
                    }else{
                        k = i;
                        break;
                    }
                }
                oper[1].regs[0] = data.regs[k];
                oper[0].mem = 1;
                oper[0].disp = data.disp;
                oper[0].scale = data.scale;
            }else if(opc == 0x16e){
                oper[0].regs[0] = data.regs[0];
                oper[1].regs[0] = data.regs[1];
				oper[1].regs[1] = data.regs[2];
				oper[1].mem = 2;
				oper[1].disp = data.disp;
				oper[1].scale = data.scale;
            }else{
				oper[0].regs[0] = data.regs[0];
                oper[1].regs[0] = data.regs[1];
			}
        }
		/*if(oper[1].uid > 0){
			lastuseb = b;
			lastusepc = firstAddr;
		}
		if(blabel[b].first == 0x5){
			if(lastuseb != NULL && lastusepc != 0){
				eachpath[pathnum].first = lastuseb;
				eachpath[pathnum].second = lastusepc;
			
				unsigned long kkk = 0;
				for (kkk; kkk <= eachpath.size(); kkk++) {
					std::cout << "path-" << kkk << ", block: ";
					std::cout << eachpath[kkk].first->start() << " pc-" << eachpath[kkk].second << std::endl;
				}
			}
			pathnum++;
		}*/
        cout << "operand 0: "<< "\n";
        for(int i = 0; i < 4; i++){
            if(oper[0].regs[i] != 0)
                printf("%d, %lx ,%s\n", i, oper[0].regs[i], oper[0].regs[i].name().c_str());
        }
        printf("mem:%d, numsrc:%d, imm:%lx, disp:%lx, scale:%lx\n",
                oper[0].mem, oper[0].numsrc, oper[0].imm, oper[0].disp, oper[0].scale);
        cout << "operand 1: "<< "\n";
        for(int i = 0; i < 4; i++){
            if(oper[1].regs[i] != 0)
                printf("%d, %lx ,%s\n", i, oper[1].regs[i], oper[1].regs[i].name().c_str());
        }
        printf("mem:%d, numsrc:%d, imm:%lx, disp:%lx, scale:%lx\n",
                oper[1].mem, oper[1].numsrc, oper[1].imm, oper[1].disp, oper[1].scale);
    }else if(size == 1){

    }else if(size == 0){

    }
    switch(opc){
        case 0x195:case 0x1bb:
            if(data.numsrc){
				managevova(oper[0], oper[1], true);
			}else{
                if(memr){
                    //add reg alias with mem
                    //addalias(operands[0], operands[1]);
					managevova(oper[0], oper[1], true);
				}else if(memw){
                    //add mem alias with reg
					managevova(oper[0], oper[1], true);
                }else{
                    // add dstreg alias with srcreg
					managevova(oper[0], oper[1], true);
                }
            }
            break;
        case 0x2c1:case 0x29c:
			managevova(oper[0], oper[1], true);
            break;
        case 0x4b://call
			if(blabel[b].first == 0xa){
				for (size_t k = 0; k < vova.size(); ++k) {
					bool jumpout = false;
					for(odata xx : vova[k]){
						if(xx.mem == 0 && xx.regs[0] == (MachRegister)0x18010007){
							tttt = xx.uid;
							jumpout = true;
							break;
						}
					}
					if(jumpout){
						break;
					}
				}
			}
			for (size_t k = 0; k < vova.size(); ++k) {
				for(odata xx : vova[k]){
					auto t3 = std::find(saved.begin(), saved.end(), xx.regs[0]);
					if (t3 == saved.end()){
						auto newEnd = std::remove(vova[k].begin(), vova[k].end(), xx);
						vova[k].erase(newEnd, vova[k].end());
					}
				}
			}
			for (size_t k = 0; k < vovi.size(); ++k) {
                for(odata xx : vovi[k]){
                    auto t3 = std::find(saved.begin(), saved.end(), xx.regs[0]);
                    if (t3 == saved.end()){
                        auto newEnd = std::remove(vovi[k].begin(), vovi[k].end(), xx);
                        vovi[k].erase(newEnd, vovi[k].end());
                    }
                }
            }
			if(blabel[b].first == 0xa){
                odata tp[2];
                memset(tp, 0, sizeof(tp));
                bool mark = false;
				tp[0].regs[0] = (MachRegister)0x18010007;
				tp[0].uid = tttt;
                tp[1].regs[0] = (MachRegister)0x18010000;
				tp[1].uid = tttt;
				for (size_t k = 0; k < vova.size(); ++k) {
					auto tx = std::find(vova[k].begin(), vova[k].end(), tp[0]);
					auto tx2 = std::find(vova[k].begin(), vova[k].end(), tp[1]);
					if (tx != vova[k].end() && tx2 == vova[k].end() && vova[k].size() > 0) {
						vova[k].push_back(tp[1]);
						mark = true;
						break;
					}else if(tx == vova[k].end() && tx2 != vova[k].end() && vova[k].size() > 0){
						vova[k].push_back(tp[0]);
						mark = true;
						break;
					}
				}
				if(!mark){
					managevova(tp[0], tp[1], true);
				}
            }else if(blabel[b].first == 0x1 || blabel[b].first == 0x4){
                odata tp[1];
                memset(tp, 0, sizeof(tp));
                tp[0].regs[0] = (MachRegister)0x18010000;
				tp[0].uid = blabel[b].second;
				std::vector<odata> atemp;
				atemp.clear();
				atemp.push_back(tp[0]);
				vova.push_back(atemp);
			}
            break;
		case 0x16e:
			managevovi(oper[0], oper[1]);
			break;
		case 0x10f:
			for (size_t k = 0; k < vova.size(); ++k) {
                for(odata xx : vova[k]){
                    auto t3 = std::find(setimul.begin(), setimul.end(), xx.regs[0]);
                    if (t3 != setimul.end()){
                        auto newEnd = std::remove(vova[k].begin(), vova[k].end(), xx);
                        vova[k].erase(newEnd, vova[k].end());
                    }
                }
            }
            for (size_t k = 0; k < vovi.size(); ++k) {
                for(odata xx : vovi[k]){
                    auto t3 = std::find(setimul.begin(), setimul.end(), xx.regs[0]);
                    if (t3 != setimul.end()){
                        auto newEnd = std::remove(vovi[k].begin(), vovi[k].end(), xx);
                        vovi[k].erase(newEnd, vovi[k].end());
                    }
                }
            }
			break;
		case 0x5e1:
			for (size_t k = 0; k < vova.size(); ++k) {
				auto x1 = std::find(vova[k].begin(), vova[k].end(), oper[0]);
				if(x1 != vova[k].end()) {
					vova[k].erase(x1);
				}
				auto xq = std::find(vova[k].begin(), vova[k].end(), oper[1]);
				if(xq != vova[k].end()) {
					vova[k].erase(xq);
				}
			}
			break;
    }
	/*for (size_t k = 0; k < vova.size(); ++k) {
		auto tx = std::find(vova[k].begin(), vova[k].end(), oper[1]);
		if (tx != vova[k].end()){
			if((*tx).uid > 0){
				lastuseb = b;
				lastusepc = firstAddr;
			}
		}
	}*/
	if(memr){
		for (size_t k = 0; k < vova.size(); ++k) {
			bool jmp1 = false;
			for(odata xx : vova[k]){
				for(int ix = 0; ix < 4; ix++){
					for(int zx = 0; zx < 4; zx++){
						if(opc == 0x6a || opc == 0x2e7){//cmp & test
							if(oper[1].regs[zx] == xx.regs[ix] && xx.uid > 0 && xx.regs[ix] != 0){
                                lastuseb = b;
                                lastusepc = firstAddr;
                                jmp1 = true;
                                break;
                            }
							if(oper[0].regs[zx] == xx.regs[ix] && xx.uid > 0 && xx.regs[ix] != 0){
                                lastuseb = b;
                                lastusepc = firstAddr;
                                jmp1 = true;
                                break;
                            }
						}else{
							if(oper[1].regs[zx] == xx.regs[ix] && xx.uid > 0 && xx.regs[ix] != 0){
								lastuseb = b;
								lastusepc = firstAddr;
								jmp1 = true;
								break;
							}
						}
					}
					if(jmp1){
						break;
					}
				}
				if(jmp1){
					break;
				}
			}
			if(jmp1){
				break;
			}
		}
	}else if(memw){
		for (size_t k = 0; k < vova.size(); ++k) {
            bool jmp1 = false;
            for(odata xx : vova[k]){
                for(int ix = 0; ix < 4; ix++){
                    for(int zx = 0; zx < 4; zx++){
                        if(oper[0].regs[zx] == xx.regs[ix] && xx.uid > 0 && xx.regs[ix] != 0){
                            lastuseb = b;
                            lastusepc = firstAddr;
                            jmp1 = true;
                            break;
                        }
                    }
                    if(jmp1){
                        break;
                    }
                }
                if(jmp1){
                    break;
                }
            }
            if(jmp1){
                break;
            }
        }
	}else if(data.mem == 2){
		for (size_t k = 0; k < vova.size(); ++k) {
            bool jmp1 = false;
            for(odata xx : vova[k]){
                for(int ix = 0; ix < 4; ix++){
                    for(int zx = 0; zx < 4; zx++){
                        if(oper[1].regs[zx] == xx.regs[ix] && xx.uid > 0 && xx.regs[ix] != 0){
                            lastuseb = b;
                            lastusepc = firstAddr;
                            jmp1 = true;
                            break;
                        }
                    }
                    if(jmp1){
                        break;
                    }
                }
                if(jmp1){
                    break;
                }
            }
            if(jmp1){
                break;
            }
        }
	}else if(data.mem == 0 && data.numsrc == false){
		for (size_t k = 0; k < vova.size(); ++k) {
            bool jmp1 = false;
            for(odata xx : vova[k]){
                for(int ix = 0; ix < 4; ix++){
                    for(int zx = 0; zx < 4; zx++){ 
                        if(oper[1].regs[zx] == xx.regs[ix] && xx.uid > 0 && xx.regs[ix] != 0){
                            lastuseb = b;
                            lastusepc = firstAddr;
                            jmp1 = true;
                            break;
                        }
                    }
                    if(jmp1){
                        break;
                    }
                }
                if(jmp1){
                    break;
                }
            }
            if(jmp1){
                break;
            }
        }
	}
	/*if(oper[1].uid > 0){
            lastuseb = b;
            lastusepc = firstAddr;
    }*/
    if(blabel[b].first == 0x5){
        if(lastuseb != NULL && lastusepc != 0){
            eachpath[pathnum].first = lastuseb;
            eachpath[pathnum].second = lastusepc;
			/*unsigned long max = eachpath.size();
            unsigned long kkk = 0;
            for (kkk = 0; kkk <= max; kkk++) {
				if(eachpath[kkk].first != NULL){
                std::cout << "path-0x" <<hex <<kkk << ", block: ";
                std::cout << eachpath[kkk].first->start() << " pc-" << eachpath[kkk].second << std::endl;
				}
            }*/
        }
        pathnum++;
    }
#if 1
    cout << "/////////////////////////////\n";
	for (size_t k = 0; k < vova.size(); ++k) {
		cout << "-------------------------\n";
		for(odata xx : vova[k]){
			for(int i = 0; i < 4; i++){
				if(xx.regs[i] != 0)
					printf("%lx ,%s\n", xx.regs[i], xx.regs[i].name().c_str());
			}
			printf("mem:%d, numsrc:%d, imm:%lx, disp:%lx, scale:%lx, uid:%lx\n",
				xx.mem, xx.numsrc, xx.imm, xx.disp, xx.scale, xx.uid);
		}
	}
	cout << "*****************************\n";
	for (size_t k = 0; k < vovi.size(); ++k) {
		cout << "-------------------------\n";
        for(odata xx : vovi[k]){
            for(int i = 0; i < 4; i++){
                if(xx.regs[i] != 0)
                    printf("%lx ,%s\n", xx.regs[i], xx.regs[i].name().c_str());
            }
            printf("mem:%d, numsrc:%d, imm:%lx, disp:%lx, scale:%lx, uid:%lx\n",
                xx.mem, xx.numsrc, xx.imm, xx.disp, xx.scale, xx.uid);
        }
    }
    cout << "/////////////////////////////\n";
#endif
}



void target_aliasanalysis(dp::Function const *f,dp::Block *b, odata data, bool memr, bool memw, unsigned long opc, int size){
#if 1	
	cout << size << "\n";
	/*for(int i = 0; i < 4; i++){
        if(data.regs[i] != 0)
            printf("%d, %lx ,%s\n", i, data.regs[i], data.regs[i].name().c_str());
    }   
    printf("memr:%d, memw:%d, mem:%d, numsrc:%d, imm:%lx, disp:%lx, scale:%lx\n", 
		memr, memw, data.mem, data.numsrc, data.imm, data.disp, data.scale);*/
#endif
	if(aset.size() == 0){
		for(dp::Edge *se : b->sources()){
			 cout <<"src---------" <<se->src()->start() <<"-"<<blabel[se->src()].first <<"\n";
		}
	}
	odata oper[4];
	memset(oper, 0, sizeof(oper));
	if(size == 3){
		
	}else if(size == 2){
		if(data.numsrc){
			if(memw){
				memcpy(oper[0].regs, data.regs, sizeof(data.regs));
				oper[0].mem = 1;
				oper[0].disp = data.disp;
				oper[0].scale = data.scale;
				oper[1].imm = data.imm;
				oper[1].numsrc = true;
			}else{
				memcpy(oper[0].regs, data.regs, sizeof(data.regs));
                oper[0].mem = 0;
				oper[1].imm = data.imm;
				oper[1].numsrc = true;
			}
		}else{
			if(memr){
				oper[0].regs[0] = data.regs[0];
				for(int i = 1; i < 4; i++){
					if(data.regs[i] != 0)
						oper[1].regs[i-1] = data.regs[i];
				}
				oper[1].mem = 1;
                oper[1].disp = data.disp;
                oper[1].scale = data.scale;
			}else if(memw){
				int k = 0;
				for(int i = 0; i < 4; i++){
                    if(data.regs[i+1] != 0){
                        oper[0].regs[i] = data.regs[i];
					}else{
						k = i;
						break;
					}
                }
				oper[1].regs[0] = data.regs[k];
				oper[0].mem = 1;
                oper[0].disp = data.disp;
                oper[0].scale = data.scale;
			}else if(opc == 0x16e){
                oper[0].regs[0] = data.regs[0];
                oper[1].regs[0] = data.regs[1];
                oper[1].regs[1] = data.regs[2];
                oper[1].mem = 2;
                oper[1].disp = data.disp;
                oper[1].scale = data.scale;                                                                                              
            }else{
				oper[0].regs[0] = data.regs[0];
				oper[1].regs[0] = data.regs[1];
			}
		}
		cout << "operand 0: "<< "\n";
		for(int i = 0; i < 4; i++){
            if(oper[0].regs[i] != 0)
				printf("%d, %lx ,%s\n", i, oper[0].regs[i], oper[0].regs[i].name().c_str());
        }
		printf("mem:%d, numsrc:%d, imm:%lx, disp:%lx, scale:%lx\n",
                oper[0].mem, oper[0].numsrc, oper[0].imm, oper[0].disp, oper[0].scale);
		cout << "operand 1: "<< "\n";
		for(int i = 0; i < 4; i++){
            if(oper[1].regs[i] != 0)
                printf("%d, %lx ,%s\n", i, oper[1].regs[i], oper[1].regs[i].name().c_str());
        }
		printf("mem:%d, numsrc:%d, imm:%lx, disp:%lx, scale:%lx\n",
                oper[1].mem, oper[1].numsrc, oper[1].imm, oper[1].disp, oper[1].scale);
	}else if(size == 1){

	}else if(size == 0){

	}
	switch(opc){
		case 0x195:case 0x1bb:
			if(data.numsrc){
				if(aset.size() != 0){
					auto t2 = std::find(aset.begin(), aset.end(), oper[0]);
					if (t2 != aset.end()) {
						aset.erase(t2);
					}
				}
			}else{
				if(memr){
					//add reg alias with mem
					//addalias(operands[0], operands[1]);
					cout << aset.size() <<"+++++++++++++++++++++"<< "\n";
					if(aset.size() != 0){
						auto it = std::find(aset.begin(), aset.end(), oper[1]);
						if (it != aset.end()) {
							std::cout << "Element found in the vector." << std::endl;
							auto t1 = std::find(aset.begin(), aset.end(), oper[0]);
							if (t1 == aset.end()) {
								oper[0].uid = (*it).uid;
								aset.push_back(oper[0]);
							}//need to remove this element from other set, but no need to consider in this one
						}else {
							std::cout << "Element not found in the vector." << std::endl;
							auto t2 = std::find(aset.begin(), aset.end(), oper[0]);
							if (t2 != aset.end()) {
                                aset.erase(t2);
							}
						}
					}
				}else if(memw){
					//add mem alias with reg
					if(aset.size() == 0){
						if(oper[1].regs[0] == 0x18010000 && size == 2 && blabel[b].first != 1){
							cout << aset.size() <<"------------------------"<< "\n";
							//oper[0].uid = blabel[b].second;
							//oper[1].uid = blabel[b].second;
							aset.push_back(oper[1]);
							aset.push_back(oper[0]);
						}
					}else{
						auto it = std::find(aset.begin(), aset.end(), oper[1]);
                        if (it != aset.end()) {
                            std::cout << "Element found in the vector." << std::endl;
                            auto t1 = std::find(aset.begin(), aset.end(), oper[0]);
                            if (t1 == aset.end()) {
								oper[0].uid = (*it).uid;
                                aset.push_back(oper[0]);
                            }//need to remove this element from other set, but no need to consider in this one
                        }else {
                            std::cout << "Element not found in the vector." << std::endl;
                            auto t2 = std::find(aset.begin(), aset.end(), oper[0]);
                            if (t2 != aset.end()) {
                                aset.erase(t2);
                            }
                        }
					}	
				}else{
					// add dstreg alias with srcreg
					if(aset.size() == 0){
						if(oper[1].regs[0] == 0x18010000 && size == 2 && blabel[b].first != 1){
                            //oper[0].uid = blabel[b].second;
                            //oper[1].uid = blabel[b].second;
							aset.push_back(oper[1]);
                            aset.push_back(oper[0]);
                        }
					}else{
						auto it = std::find(aset.begin(), aset.end(), oper[1]);
                        if (it != aset.end()) {
                            std::cout << "Element found in the vector." << std::endl;
                            auto t1 = std::find(aset.begin(), aset.end(), oper[0]);
                            if (t1 == aset.end()) {
                                oper[0].uid = (*it).uid;
								aset.push_back(oper[0]);
                            }//need to remove this element from other set, but no need to consider in this one
                        }else {
                            std::cout << "Element not found in the vector." << std::endl;
                            auto t2 = std::find(aset.begin(), aset.end(), oper[0]);
                            if (t2 != aset.end()) {
                                aset.erase(t2);
                            }
                        }
					}
				}
			}
			break;
		case 0x10f:case 0x2c1:case 0x29c:
			break;
		case 0x4b:
			for(odata xx : aset){
                auto t3 = std::find(saved.begin(), saved.end(), xx.regs[0]);
                if (t3 == saved.end()){
                    auto newEnd = std::remove(aset.begin(), aset.end(), xx); 
                    aset.erase(newEnd, aset.end());
                }    
            }
			if(blabel[b].first == 0xffff){
				for(odata xx : aset){
					if(xx.regs[0] == 0x18010007 && xx.mem == 0){
						cout << "alloc - dealloc paired\n";
					}else{
						cout << "not paired\n";
						notpair = true;
					}
				}
			}else if(blabel[b].first == 1 || blabel[b].first == 4){
				odata tpc[1];
				memset(tpc, 0, sizeof(tpc));
				tpc[0].regs[0] = (MachRegister)0x18010000;
				tpc[0].uid = blabel[b].second;
				if(aset.size() == 0){
					aset.push_back(tpc[0]);
				}else{
					auto tx = std::find(aset.begin(), aset.end(), tpc[0]);
					if (tx != aset.end()) {
						aset.erase(tx);
					}
					aset.push_back(tpc[0]);
				}
			}
			/*special handle for memset*/
			if(blabel[b].first == 0xa){
				odata tp[2];
				memset(oper, 0, sizeof(tp));
				tp[0].regs[0] = (MachRegister)0x18010007;
				tp[1].regs[0] = (MachRegister)0x18010000;
				auto tx = std::find(aset.begin(), aset.end(), tp[0]);
				if (tx == aset.end()) {
					aset.push_back(tp[0]);
				}
				auto tx2 = std::find(aset.begin(), aset.end(), tp[1]);
				if (tx2 == aset.end()) {
					aset.push_back(tp[1]);
				}
			}
			break;
	}
#if 1
	cout << "/////////////////////////////\n";
	for(odata xx : aset){
		for(int i = 0; i < 4; i++){
            if(xx.regs[i] != 0)
                printf("%lx ,%s\n", xx.regs[i], xx.regs[i].name().c_str());
        }
        printf("mem:%d, numsrc:%d, imm:%lx, disp:%lx, scale:%lx, uid:%lx\n",
			xx.mem, xx.numsrc, xx.imm, xx.disp, xx.scale, xx.uid);
	}
	cout << "/////////////////////////////\n";
#endif
}

//void findleakpath

void operandanalysis(dp::Function const *f, dp::Block *b, Dyninst::InstructionAPI::Instruction ins, 
Dyninst::InstructionAPI::InstructionDecoder decoder, bool fullcheck, 
Dyninst::Address firstAddr, dp::Block *xxb, bool findp){
	Expression::Ptr check;
    std::vector<Operand> operands;
	odata data;
	Dyninst::InstructionAPI::Expression::Ptr express; 
	std::vector<Expression::Ptr> kid0, kid1, kid2, kid3, kid4, kid5, kid6, kid7, kid8, kid9;
	ins.getOperands(operands);
	bool numsrc = false;
	bool memr = false;
	bool memw = false;
	bool isfs = false;
	//bool stop = false;
	MachRegister regs[4] = {(Dyninst::MachRegister)0};
	long scale, disp, imm;
	scale = disp = imm = 0;
	//cout << operands.size()<< "\n";
	int pos = 0;
	std::set<InstructionAPI::RegisterAST::Ptr> regsRead;
	regsRead.clear();
	//cout <<"\n" <<hex << ins.getOperation().getID() <<" "<< ins.format() << "\""<<"\n";
	ins.getReadSet(regsRead);
	for(InstructionAPI::RegisterAST::Ptr zz : regsRead){
		MachRegister reg = zz->getID();
		//printf("x------%lx ,%s\n", reg, reg.name().c_str());
		if(reg == (MachRegister)0x18020002){
			isfs = true;
		}
	}
	//MachRegister reg2 = (MachRegister)0x18020001;
	//printf("qqq------%lx ,%s\n", reg2, reg2.name().c_str());
	if(ins.readsMemory()){
		memr = true;
	}else if(ins.writesMemory()){
		memw = true;
	}
	unsigned long opc = ins.getOperation().getID();
	for (Operand opd : operands) {
		kid0.clear();
		kid1.clear();
		kid2.clear();
		kid3.clear();
#if DB
		cout << "******" <<opd.format(f->region()->getArch()) << "******\n";
#endif		
		express = opd.getValue();
		express->getChildren(kid0);
		if(kid0.size() == 0){
			check = express;
            if(dynamic_cast<RegisterAST*>(check.get())){
                MachRegister reg = (boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(check))->getID();
				regs[pos] = unif(reg);
				if (pos < 4){
					if(regs[pos] != 0){
					pos ++;
					}
				}else{
					exit(-1);
				}
#if DB                
				printf("1------%lx ,%s\n", reg, reg.name().c_str());
#endif            
			}else{
				numsrc = true;
				imm = (check)->eval().convert<long>();
#if DB                
				printf("2------%lx ", (check)->eval().convert<long>());
#endif            
			}
		}else{
			for (Expression::Ptr ops : kid0){
				ops->getChildren(kid1);
				if(kid1.size() == 2){
					for(Expression::Ptr sop : kid1){
						sop->getChildren(kid2);
#if DB
						cout <<"^^^^^^^"<< kid2.size()<<"\n";
#endif						
						if(kid2.size() == 2){
							cout << "new case need handle 1" << "\n";
							for(Expression::Ptr sop2 : kid2){
								sop2->getChildren(kid3);
								if(kid3.size() == 2){

								}else if(kid3.size() == 1){
									
								}else if(kid3.size() == 0){
									if(dynamic_cast<RegisterAST*>(sop2.get())){
										MachRegister reg = (boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(sop2))->getID();
										regs[pos] = unif(reg);
										if (pos < 4){
											if(regs[pos] != 0){
												pos ++;
											}
										}else{
											exit(-1);
										}

										printf("9------%lx ,%s\n", reg, reg.name().c_str());
									}else{
										scale = (sop2)->eval().convert<long>();
										printf("10------%lx\n", (sop2)->eval().convert<long>());
									}
								}
							}
							//stop = true;
							//exit(-1);
						}else if(kid2.size() == 1){
							cout << "new case need handle 2" << "\n";
							exit(-1);
						}else if(kid2.size() == 0){
							check = sop;
							if(dynamic_cast<RegisterAST*>(check.get())){
								MachRegister reg = (boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(check))->getID();
								regs[pos] = unif(reg);
								if (pos < 4){
									if(regs[pos] != 0){
										pos ++;
									}
								}else{
									exit(-1);
								}
#if DB								
								printf("3------%lx ,%s\n", reg, reg.name().c_str());
#endif							
							}else{
								disp = (check)->eval().convert<long>();
#if DB								
								printf("4------%lx\n", (check)->eval().convert<long>());
#endif							
							}
						}
					}
				}else if(kid1.size() == 1){
					check = ops;
					if(dynamic_cast<RegisterAST*>(check.get())){
						MachRegister reg = (boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(check))->getID();
#if DB						
						printf("5------%lx ,%s\n", reg, reg.name().c_str());
#endif					
					}else{
#if DB						
						printf("6------%lx\n", (check)->eval().convert<long>());
#endif					
					}
				}else if(kid1.size() == 0){
					check = ops;
					if(dynamic_cast<RegisterAST*>(check.get())){
						MachRegister reg = (boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(check))->getID();
						regs[pos] = unif(reg);
						if (pos < 4){
							if(regs[pos] != 0){
								pos ++;
							}
						}else{
							exit(-1);
						}
#if DB						
						printf("7------%lx ,%s\n", reg, reg.name().c_str());
#endif					
					}else{
						disp = (check)->eval().convert<long>();
#if DB						
						printf("8------%lx\n", (check)->eval().convert<long>());
#endif					
					}
				}	
			}
		}
		/*if (pos < 4){
			if(regs[pos] != 0){
				pos ++;
			}
		}*/
	}
	/*for(int i = 0; i < 4; i++){
		if(regs[i] != 0)
			printf("%d, %lx ,%s\n", i,regs[i], regs[i].name().c_str());
	}
	printf("memr:%d, memw:%d, numsrc:%d, imm:%lx, disp:%lx\n", memr, memw, numsrc, imm,disp);*/
	//if(stop)
		//exit(-1);
	memcpy(data.regs, regs, sizeof(regs));
    if(isfs){
		if(numsrc){
			data.regs[0] = (MachRegister)0x18020002;
		}else{
			if(memr){
				data.regs[1] = (MachRegister)0x18020002;
			}else if(memw){
				data.regs[0] = (MachRegister)0x18020002;
			}
		}
	}
	data.numsrc = numsrc;
    if(memr || memw){
        data.mem = 1;
    }else{
		if(opc == 0x16e){
			data.mem = 2;
		}else{
			data.mem = 0;
		}
    }
	//cout << "--------disp: "<< disp << "\n";
    data.disp = disp;
	//cout << "--------data.disp: "<< data.disp << "\n";
    data.imm = imm;
    data.scale = scale;
	for(int i = 0; i < 4; i++){
        if(data.regs[i] != 0)
            printf("%d, %lx ,%s\n", i, data.regs[i], data.regs[i].name().c_str());
    }
    printf("memr:%d, memw:%d, numsrc:%d, imm:%lx, disp:%lx\n", memr, memw, data.numsrc, data.imm, data.disp);
	if(fullcheck){
		full_aliasanalysis(f, b, data, memr, memw ,opc, operands.size(), firstAddr, xxb, findp);
	}else{
		target_aliasanalysis(f, b, data, memr, memw ,opc, operands.size());
	}
}

bool taintanalysis(dp::Function const *f, dp::Block *b, bool fullcheck, dp::Block* xxb, bool findp){
	Dyninst::InstructionAPI::Instruction ins;
	Dyninst::InstructionAPI::InstructionDecoder decoder(f->isrc()->getPtrToInstruction(f->addr()),
        Dyninst::InstructionAPI::InstructionDecoder::maxInstructionLength,
        f->region()->getArch());
	Dyninst::Address firstAddr = b->start();
    Dyninst::Address lastAddr = b->end();
	while(firstAddr < lastAddr){
		ins = decoder.decode((unsigned char *)f->isrc()->getPtrToInstruction(firstAddr));
#if 1
		cout <<"\n" <<hex << firstAddr <<": \"" << ins.getOperation().getID() <<" "<< ins.format() << "\""<<"\n";
#endif		
		operandanalysis(f, b, ins, decoder, fullcheck, firstAddr, xxb, findp);
		firstAddr += ins.size();
	}
	return false;
}

/*bool tracepath(dp::Function const *f, dp::Block *b, std::unordered_set<Dyninst::Address> *used){
	int func = 0;
	dp::Edge *edge = NULL;
	bool iscall = false;
	if(b->start() == f->entry()->start()){
		//taintanalysis(f, b);
		used->insert(f->entry()->start());
	}
	for (dp::Edge *e : b->targets()) {
		if (!e)
            continue;
        cout <<"$$$$$$$$$$$$$$$$" <<e->trg()->start()<<"\n";
		if (e->type() == dp::CALL){
			cout <<"################" <<e->trg()->start()<<"\n";
			func = calltype(all, e->trg()->start());
			printf("call type is %d\n", func);
			//if(func == 0){
                //exit(-1);
            //}else{
				iscall = true;
			//}
			continue;
		}else if(e->type() == dp::RET){
			cout <<"----------------" <<e->trg()->start()<<"\n";
			continue;
		}
		edge = e;
	}
	//cout <<"^^^^^^^^^^^" <<edge->trg()->start()<<"\n";
	if(iscall){
		if(!(edge->interproc())){
			used->insert(edge->trg()->start());
			//taintanalysis(f, edge->trg());
			tracepath(f, edge->trg(), used);
		}
	}else{
		for (dp::Edge *ed : b->targets()) {
			if (!ed)
				continue;
			if(!(ed->interproc())){
				cout << "---------------------------------" << "\n";
				//taintanalysis(f, ed->trg());
				used->insert(ed->trg()->start());
				tracepath(f, ed->trg(), used);
			}
			//used->insert(ed->trg()->start());
		}
	}
	return false;
}*/

bool hasdup(dp::Function const *f, std::vector<dp::Block*>& vec) {
    std::unordered_set<Dyninst::Address> seen1;
    seen1.clear();
	std::vector<dp::Loop *> lps;
	lps.clear();
	f->getLoops(lps);
	for (dp::Block* num : vec) {
		for (dp::Loop* lp : lps) {
			if(lp->hasBlock(num)){
				if (seen1.count(num->start()) > 1) {
					return true; 
				}
			}
		}
		if (seen1.count(num->start()) > 0) {
            return true; 
        }
        seen1.insert(num->start());
    }
    return false;
}

void findallPaths(dp::Function const *f,dp::Block *root, std::vector<dp::Block*> &path, dp::Block *xxb, bool findp) {
    //dp::Edge *edge = NULL;
	bool iscall = false;
	bool isret = false;
	bool fullcheck = false;
	int func = 0;
	if (root == nullptr) {
        return;
    }
	
	
    // Add current node to the path
    
	path.push_back(root);
	if(hasdup(f,path)){
		path.pop_back();
		return;
	}
    // If it's a leaf node, print the path
    if (root->targets().size() == 0) {
        for (int i = 0; i < path.size() - 1; ++i) {
            cout <<std::hex <<path[i] << " -> ";
        }
        cout << path.back() << endl;
    }
	
	for (dp::Edge *e : root->targets()) {
        if (!e)
            continue;
        if (e->type() == dp::CALL){
            func = calltype(all, e->trg()->start());
            printf("call type is %d, 0x%lx\n", func, e->trg()->start());
            /*if(func == 1){
				blabel[root] = 1;
            }else if(func ==4){
				blabel[root] = 4;
			}*/
			if(func == 1 || func ==4 || func == 2){
				blabel[root].first = func;
				blabel[root].second = uid;
				uid++;
			}else{
				blabel[root].first = func;
				blabel[root].second = 0;
			}
            iscall = true;
            continue;
        }else if(e->type() == dp::RET){
			isret = true;
			blabel[root].first = 5;
            continue;
        }
		//printf("-----0x%lx\n", e->trg()->start());
        //edge = e;
    }
    if(iscall){
		for (dp::Edge *ced : root->targets()) {
			if(!(ced->interproc()))
				findallPaths(f,ced->trg(), path, xxb, findp);
		}
        //if(!(edge->interproc())){
          //  findallPaths(f, edge->trg(), path);
        //}    
    }else if(isret){
		int m, fr;
		m = fr = 0;
		for (int i = 0; i < path.size() - 1; ++i) {
            cout << std::hex <<path[i]->start() <<"-" << blabel[path[i]].first<<"-"<<blabel[path[i]].second << " -> ";
			if(blabel[path[i]].first == 1 || blabel[path[i]].first == 4){//only fit to single obj
				m = i;
			}else if(blabel[path[i]].first == 0xffff){
				fr = i;
			}
        }
		cout << path.back()->start() <<"-" << blabel[path.back()].first <<endl;
		//printf("m=%d, fr=%d\n", m, fr);
		if(fr > m){
			aset.clear();
			for (int k = m; k <= fr; k++){
				taintanalysis(f, path[k], fullcheck, xxb, findp);
				
				cout << std::hex <<path[k]->start() <<"-" << blabel[path[k]].first << " -> ";
			}
			cout << "\n";
			if(notpair){
				fullcheck = true;
				vova.clear();
				vovi.clear();
				for (int j = 0; j < path.size(); ++j) {
					taintanalysis(f, path[j], fullcheck, xxb, findp);
				}
			}
		}else{
			//no free call, leak or pointer escape
			fullcheck = true;
			vova.clear();
			vovi.clear();
			lastuseb = NULL;
			lastusepc = 0x0;
			fiteachpath = false;
			for (int j = 0; j < path.size(); ++j) {
				taintanalysis(f, path[j], fullcheck, xxb, findp);
			}
		}        
	}else{
        for (dp::Edge *ed : root->targets()) {
			findallPaths(f,ed->trg(), path, xxb, findp);
		}
    }

    // Backtrack: remove the current node from the path
    path.pop_back();
}

void getloopchild(dp::LoopTreeNode *ls){
	std::vector<dp::LoopTreeNode *> lc;
	lc.clear();
	lc = ls->children;
	std::vector<dp::Block *> lbs;
	std::vector<dp::Block *> ebs;
	//std::vector<dp::Block *> ubs;
	lbs.clear();
	ebs.clear();
	//ubs.clear();
	if(lc.size() > 0){
		for (dp::LoopTreeNode* ln : lc) {
			lbs.clear();
			ebs.clear();
			std::vector<dp::LoopTreeNode *> ltemp;
			ltemp.clear();
			ltemp = ln->children;
			if(ltemp.size() > 0){
				lbs.clear();
				ln->loop->getLoopBasicBlocksExclusive(lbs);
				//cout << "$$$$$$$$$$$$$$$$$$$$\n";
				ln->loop->getLoopEntries(ebs);
				for (dp::Block *eb : ebs) {
					//cout << "ebtry block "<<hex <<eb->start() << "\n";
					if (std::find(ubs.begin(), ubs.end(), eb) == ubs.end()) {
						ubs.push_back(eb);
					}
				}
				//cout << ".....................\n";
				for (dp::Block *bb : lbs) {
					//cout << "block "<<hex <<bb->start() << "\n";
				}
				//cout << "$$$$$$$$$$$$$$$$$$$$\n";
				getloopchild(ln);
			}else{
				//cout << "^^^^^^^^^^^^^^^^^^^^\n";
				ln->loop->getLoopBasicBlocks(lbs);
				ln->loop->getLoopEntries(ebs);
				for (dp::Block *eb : ebs) {
                    //cout << "ebtry block "<<hex <<eb->start() << "\n";
					if (std::find(ubs.begin(), ubs.end(), eb) == ubs.end()) {
                        ubs.push_back(eb);
                    }
                }
                //cout << ".....................\n";
				for (dp::Block *bb : lbs) {
					//cout << "block "<<hex <<bb->start() << "\n";
				}
				//cout << "^^^^^^^^^^^^^^^^^^^^\n";
			}
		}
	}else{
		dp::Loop *lo = ls->loop;
		if(lo != NULL){
			lo->getLoopBasicBlocks(lbs);
			//cout << "********************\n";
			for (dp::Block *bb : lbs) {
				cout << "block "<<hex <<bb->start() << "\n";
			}    
			//cout << "********************\n";
		}
	}
	/*for (dp::Block *ub : ubs) {
		cout << "entry block "<<hex <<ub->start() << "\n";
	}*/
}

Dyninst::Address findfixpoint(dp::Function const *f,dp::Block* b, Dyninst::Address pc){
	//cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
	bool inloop = false;
	std::vector<dp::Block *> lbs;
	dp::LoopTreeNode* ll = f->getLoopTree();
    std::vector<dp::LoopTreeNode *> lc;
    lc.clear();
    lc = ll->children;
    lbs.clear();
    //getloopchild(ll);
	std::vector<dp::Loop *> lps;
	std::vector<dp::Loop *> outs;
    lps.clear();
	outs.clear();
    f->getLoops(lps);
	for (dp::Loop* lp : lps) {
		if(lp->hasBlock(b)){
			inloop = true;
			break;
		}
	}
	if(inloop){
		getloopchild(ll);
	}else{
		if (std::find(ubs.begin(), ubs.end(), b) == ubs.end()) {
			ubs.push_back(b);
		}
	}
	//cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
}


int main(int argc, char *argv[]) {
	if (argc < 2 || argc > 3) {
		std::cerr << "Usage: " << argv[0] << " executable\n";
		return -1;
	}
	int hasloop = 0;
	auto *sts = new dp::SymtabCodeSource(argv[1]);
	auto *co = new dp::CodeObject(sts);
	Dyninst::InstructionAPI::Instruction instr;

	// Parse the binary
	co->parse();
  	cout << "digraph G {" << '\n';

  	// Print the control flow graph
	all = co->funcs();

	// Remove compiler-generated and system functions
	{
		auto ignore = [&all](dp::Function const *f) {
		auto const &name = f->name();
		bool const starts_with_underscore = name[0] == '_';
		bool const ends_with_underscore = name[name.length() - 1] == '_';
		bool const is_dummy = name == "frame_dummy";
		bool const is_clones = name.find("tm_clones") != std::string::npos;
		return starts_with_underscore || ends_with_underscore || is_dummy ||
             is_clones;
		};

		// 'funclist' is a std::set which has only const iterators
		auto i = all.begin();
		while (i != all.end()) {
			if (ignore(*i)) {
				i = all.erase(i);
			} else {
				++i;
			}
		}
	}

	std::unordered_set<Dyninst::Address> seen;

	int cluster_index = 0;
	for (auto const *f : all) {
		std::vector<dp::Block *> path;
		std::vector<dp::Loop *> loops;
		std::vector<dp::Edge *> bedges;
		std::vector<dp::Block *> lbs;
		std::vector<dp::Block *> postbb;
		infomap.clear();
		path.clear();
		blabel.clear();
		vova.clear();
		vovi.clear();
		loops.clear();
		bedges.clear();
		eachpath.clear();
		ubs.clear();
		postbb.clear();
		Dyninst::InstructionAPI::InstructionDecoder decoder(f->isrc()->
                                            getPtrToInstruction(f->addr()),
        Dyninst::InstructionAPI::InstructionDecoder::maxInstructionLength,
        f->region()->getArch());
		// Make a cluster for nodes of this function
		std::cout << "\t subgraph cluster_" << cluster_index << " { \n\t\t label=\""
              << f->name() << "\"; \n\t\t color=blue;" << '\n';

		std::cout << "\t\t\"" << std::hex << f->addr() << std::dec
              << "\" [shape=box";

		if (f->retstatus() == dp::NORETURN)
			std::cout << ",color=red";

		std::cout << "]" << '\n';

		// Label functions by name
		std::cout << "\t\t\"" << std::hex << f->addr() << std::dec
              << "\" [label = \"" << f->name() << "\\n"
              << std::hex << f->addr() << std::dec << "\"];" << '\n';

		std::stringstream edgeoutput;
		/*dp::LoopTreeNode* ll = f->getLoopTree();
        std::vector<dp::LoopTreeNode *> lc;
        lc.clear();
        lc = ll->children;
        lbs.clear();*/
		//getloopchild(ll);
		
		findallPaths(f, f->entry(), path, NULL, false);
		unsigned long max = eachpath.size();
		unsigned long kkk = 0;
        for (kkk = 0; kkk <= max; kkk++) {
            if(eachpath[kkk].first != NULL){
				std::cout << "path-0x" <<hex <<kkk << ", block: ";
                std::cout << eachpath[kkk].first->start() << " pc-" << eachpath[kkk].second << std::endl;
            }
        }
		std::unordered_set<std::pair<dp::Block*, Dyninst::Address>, PairHash, PairEqual> uniquePairs;

		// Iterate over each element of the original map
		for (const auto& elem : eachpath) {
        // Insert the pair into the unordered set
			uniquePairs.insert(elem.second);
		}

		// Print out the unique pairs
		for (const auto& pair : uniquePairs) {
			if(pair.first != NULL){
				std::cout << "Unique Pair: (" << pair.first->start() << ", " << pair.second << ")\n";
				findfixpoint(f, pair.first, pair.second);
			}
		}

		for (dp::Block *fb : f->blocks()) {
			bool isdom = true;
			//cout << "entry block "<<hex <<fb->start() << "\n";
			for (dp::Block *uub : ubs) {
				if(uub == fb)
					continue;
				if(!f->postDominates(fb, uub)){
					isdom = false;
					break;
				}
			}
			if(isdom){
				if (std::find(postbb.begin(), postbb.end(), fb) == postbb.end()) {
					postbb.push_back(fb);
				}
			}
		}
		path.clear();
		for (dp::Block *pb : postbb) {
			bool isend = false;
			cout << "possible insert block "<<hex <<pb->start() << "\n";
			findallPaths(f, f->entry(), path, pb, true);
			for(odata ww : infomap[pb]){
				cout <<"Safe insert point: ";
				cout <<pb->start()<<"--"<< "operand data:" << "\n";
				for(int i = 0; i < 4; i++){
					if(ww.regs[i] != 0)
						printf("%d, %lx ,%s\n", i, ww.regs[i], ww.regs[i].name().c_str());
				}    
				printf("mem:%d, numsrc:%d, imm:%lx, disp:%lx\n", ww.mem, ww.numsrc, ww.imm, ww.disp);
				isend = true;
			}
			if(isend)
				break;
		}

	}
  std::cout << "}" << '\n';
}
