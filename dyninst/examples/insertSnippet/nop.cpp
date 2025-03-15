#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "BPatch_object.h"
#include "BPatch_point.h"
#include "PatchMgr.h"

#include <vector>

namespace dpa = Dyninst::PatchAPI;

class NoopSnippet : public dpa::Snippet {
public:
	bool generate(dpa::Point *pt, Dyninst::Buffer &buffer) override {
		uint8_t byte = 0x90;
		std::cout << "inserting a no op @" <<std::hex<< pt->block()->start() << std::endl;
		for (int i = 0; i < 10; i++) {
			buffer.push_back(byte);
		}
		return true;
	}
};

int main(int argc, const char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage:\n\t" << argv[0]
              << " <input binary> <output binary path>\n";
		return -1;
	}

	const char *input_binary = argv[1];
	const char *output_binary = argv[2];

	BPatch bpatch;
	BPatch_binaryEdit *app = bpatch.openBinary(input_binary, false);
	if (!app) {
		std::cerr << "Unable to open '" << input_binary << "'\n";
		return -1;
	}

	BPatch_image *image = app->getImage();
	if (!image) {
		std::cerr << "Failed to get image\n";
		return -1;
	}

	dpa::PatchMgrPtr patchMgr = dpa::convert(image);

	vector<BPatch_object *> objects;
	image->getObjects(objects);
	std::cout << "Found " << objects.size() << " objects\n";
	BPatch_object *batchObj = objects[0];
	dpa::PatchObject *binobj = dpa::convert(batchObj);

	dpa::Patcher patcher(patchMgr);

	NoopSnippet::Ptr snippet = NoopSnippet::create(new NoopSnippet);

	std::vector<dpa::PatchFunction *> functions;
	binobj->funcs(std::back_inserter(functions));

	for (dpa::PatchFunction *fun : functions) {
		std::vector<dpa::Point *> f_entryPoints;
		patchMgr->findPoints(dpa::Scope(fun), dpa::Point::PreInsn,back_inserter(f_entryPoints));
		std::cout << fun->name() << " has:\n\t" << f_entryPoints.size()
              << " instruction" << std::endl;

		for (dpa::Point *point : f_entryPoints) {
			//std::cerr << "Patching @ " <<std::hex<< point->addr() << std::endl;
			if(point->addr() == 0x12a4){
				std::cerr << "Patching @ " <<std::hex<< point->addr() << std::endl;
				patcher.add(dpa::PushBackCommand::create(point, snippet));
			}
		}
	}
	patcher.commit();
	app->writeFile(output_binary);
	std::cout << "Done\n";
}
