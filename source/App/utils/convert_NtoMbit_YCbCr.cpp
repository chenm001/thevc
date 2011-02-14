
#include <cstdlib>

#include "TLibCommon/TComPicYuv.h"
#include "TLibVideoIO/TVideoIOYuv.h"
#include "../../App/TAppCommon/program_options_lite.h"

using namespace std;
namespace po = df::program_options_lite;

int main(int argc, const char** argv)
{
	bool do_help;
	string filename_in, filename_out;
	unsigned int width, height;
	unsigned int bitdepth_in, bitdepth_out;
	unsigned int num_frames;

	po::Options opts;
	opts.addOptions()
	("help", do_help, false, "this help text")
	("InputFile,i", filename_in, string(""), "input file to convert")
	("OutputFile,o", filename_out, string(""), "output file")
	("SourceWidth", width, 0u, "source picture width")
	("SourceHeight", height, 0u, "source picture height")
	("InputBitDepth", bitdepth_in, 8u, "bit-depth of input file")
	("OutputBitDepth", bitdepth_out, 8u, "bit-depth of output file")
	("NumFrames", num_frames, 0xffffffffu, "number of frames to process")
	;

	po::setDefaults(opts);
	po::scanArgv(opts, argc, argv);

	if (argc == 1 || do_help)
	{
		/* argc == 1: no options have been specified */
		po::doHelp(cout, opts);
		return EXIT_FAILURE;
	}

	TVideoIOYuv input;
	TVideoIOYuv output;

	input.open((char*)filename_in.c_str(), false, bitdepth_in, bitdepth_out);
	output.open((char*)filename_out.c_str(), true, bitdepth_out, bitdepth_out);

	TComPicYuv frame;
	frame.create( width, height, 1, 1, 0 );

	int pad[2] = {0, 0};

	unsigned int num_frames_processed = 0;
	while (!input.isEof()) {
		TComPicYuv *stupid = &frame;
		input.read(stupid, pad);

#if 0
		Pel* img = frame.getLumaAddr();
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < height; x++)
				img[x] = 0;
			img += frame.getStride();
		}
		img = frame.getLumaAddr();
		img[0] = 1;
#endif

		output.write(&frame, pad);
		num_frames_processed++;
		if (num_frames_processed == num_frames)
			break;
	}

	input.close();
	output.close();

	return EXIT_SUCCESS;
}
