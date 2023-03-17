#include <string>
#include <vector>
#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>
#include <opencv2/opencv.hpp>

#include "types.h"

int run_client(const std::string& tcp_address) {

	RecordPacket record_packet;

	int wait_timer = 1000 / record_packet.desiredFPS;

	nng_socket socket;
	nng_dialer dialer;
	int nng_result = 0;
	if ((nng_result = nng_sub0_open(&socket)) != 0) {
		return -1;
	}
	if ((nng_result = nng_dial(socket, tcp_address.c_str(), &dialer, 0)) != 0) {
		nng_close(socket);
		return -2;
	}

	std::vector<cv::VideoCapture> cams;
	std::vector<cv::VideoWriter> files;

	while (true) {
		int idx = cams.size();
		cv::VideoCapture cam(idx, cv::CAP_DSHOW);
		if (!cam.isOpened()) {
			break;
		}
		cams.emplace_back(cam);
	}

	std::vector<cv::Mat> frames(cams.size());

	std::cout << "All cameras open!" << std::endl;

	while (true) {

		bool wasRecording = record_packet.record != 0;
		size_t packetSize = 0;
		auto nng_result = nng_recv(socket, &record_packet, &packetSize, NNG_FLAG_NONBLOCK);
		if (packetSize != 0) {

			// record or stop recording?
			wait_timer = 1000 / record_packet.desiredFPS;

			if (wasRecording &&
				record_packet.record == 0) {
				std::cout << "Stopping recording..." << std::endl;

				for (auto& file : files)
					file.release();
				files.clear();
			}
			if (!wasRecording &&
				record_packet.record != 0) {
				std::cout << "Starting recording..." << std::endl;

				for (int idx = 0; auto & cam : cams) {
					int width = (int)cam.get(cv::CAP_PROP_FRAME_WIDTH);
					int height = (int)cam.get(cv::CAP_PROP_FRAME_HEIGHT);

					auto file_path = std::format("{}{}.avi", record_packet.prefixPath, idx);
					if (idx < 10)
						file_path = std::format("{}0{}.avi", record_packet.prefixPath, idx);

					files.push_back(cv::VideoWriter(file_path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), record_packet.desiredFPS, cv::Size(width, height)));
					idx++;
				}
			}

			if (record_packet.close != 0) {
				std::cout << "Shutting down..." << std::endl;

				for (auto& file : files)
					file.release();
				files.clear();
				break;
			}
		}

		if (record_packet.record != 0) {
			for (int idx = 0; auto & cam : cams) {

				cam.read(frames[idx]);
				files[idx].write(frames[idx]);
				idx++;
			}

			cv::waitKey(wait_timer);
		}
	}


	for (auto& cam : cams)
		cam.release();

	nng_close(socket);

	return 0;
}