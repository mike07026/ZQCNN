#include "ZQ_FaceIDPrecisionEvaluation.h"
#include "ZQ_FaceRecognizerSphereFaceZQCNN.h"
#include "ZQ_CNN_CompileConfig.h"
#if ZQ_CNN_USE_BLAS_GEMM
#include <openblas\cblas.h>
#pragma comment(lib,"libopenblas.lib")
#elif ZQ_CNN_USE_MKL_GEMM
#include <mkl\mkl.h>
#pragma comment(lib,"mklml.lib")
#endif

using namespace std;
using namespace ZQ;

bool EvaluationSphereFaceZQCNNOnLFW(const std::string& prototxt_file, const std::string& caffemodel_file, const std::string& out_blob_name,
	const std::string& list_file, const std::string& folder, int max_thread_num = 4, bool use_flip = false)
{
	int real_num_threads = __max(1, __min(max_thread_num, omp_get_num_procs() - 1));

	std::vector<ZQ_FaceRecognizer*> recognizers(real_num_threads);

	for (int i = 0; i < real_num_threads; i++)
	{
		recognizers[i] = new ZQ_FaceRecognizerSphereFaceZQCNN();
		if (!recognizers[i]->Init("", prototxt_file, caffemodel_file, out_blob_name))
		{
			printf("failed to load sphereface prototxt: %s, caffemodel %s\n", prototxt_file.c_str(), caffemodel_file.c_str());
			return false;
		}
	}
	printf("load sphereface done!\n");

	return ZQ_FaceIDPrecisionEvaluation::EvaluationOnLFW(recognizers, list_file, folder, use_flip);
}


int main(int argc, const char** argv)
{
	int max_thread_num = 4;
	bool use_flip = true;
	if (argc < 6)
	{
		printf("%s prototxt_file caffemodel_file out_blob_name list_file folder [max_thread_num] [use_flip]\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc > 6)
		max_thread_num = atoi(argv[6]);
	if (argc > 7)
		use_flip = atoi(argv[7]);

#if ZQ_CNN_USE_BLAS_GEMM
	openblas_set_num_threads(1);
#elif ZQ_CNN_USE_MKL_GEMM
	mkl_set_num_threads(1);
#endif

	double t1 = omp_get_wtime();
	if (!EvaluationSphereFaceZQCNNOnLFW(string(argv[1]), string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), max_thread_num, use_flip))
		return EXIT_FAILURE;
	double t2 = omp_get_wtime();
	printf("total cost: %.3f secs\n", t2 - t1);
	return EXIT_SUCCESS;
}