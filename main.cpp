#include <opencv2/opencv.hpp>
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <vector>

int main(int argc, char* argv[]) {

	std::vector<std::string> client_addresses;

	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "--clients") == 0) {

			// split clients by ,

		}
	}

	// open socket to receive record commands

	std::vector<cv::VideoCapture> cams;
	std::vector<cv::VideoWriter> files;

	while (true) {

		int idx = cams.size();

		cv::VideoCapture cam(idx, cv::CAP_DSHOW);

		if (!cam.isOpened()) {
			break;
		}

		cams.emplace_back(cam);

		int width = (int)cam.get(cv::CAP_PROP_FRAME_WIDTH);
		int height = (int)cam.get(cv::CAP_PROP_FRAME_HEIGHT);

		auto file_path = std::format("cam_{}.avi", idx);
		if (idx < 10)
			file_path = std::format("cam_0{}.avi", idx);
		
		files.push_back(cv::VideoWriter(file_path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 15, cv::Size(width, height)));
	}

	std::vector<cv::Mat> frames(cams.size());

	std::cout << "All cameras open!" << std::endl;

	while (true) {

		for (int j = 0; auto & cam : cams) {

			cam.read(frames[j]);
			files[j].write(frames[j]);
			cv::imshow(std::format("Cam {}", j), frames[j]);
			j++;
		}

		if (cv::waitKey(33) == 'q')
			break;
	}

	for (auto& file : files)
		file.release();

	for (auto& cam : cams)
		cam.release();

	cv::destroyAllWindows();
}