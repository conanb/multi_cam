#include <opencv2/opencv.hpp>
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <vector>
#include <set>
#include <algorithm>

int run_host(const std::string& tcp_address);
int run_client(const std::string& tcp_address);

int main(int argc, char* argv[]) {

	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i)
		args.push_back(argv[i]);

	bool isClient = std::find(args.begin(), args.end(), std::string("--client")) != args.end();

	std::string address = "127.0.0.1";
	int port = 7788;

	auto address_iter = std::find(args.begin(), args.end(), std::string("--address"));
	if (address_iter != args.end()) {
		address_iter++;
		address = *address_iter;
	}

	auto port_iter = std::find(args.begin(), args.end(), std::string("--port"));
	if (port_iter != args.end()) {
		port_iter++;
		port = std::atoi(port_iter->c_str());
	}

	auto tcp_address = std::format("tcp://{}:{}", address, port);

	if (isClient)
		return run_client(tcp_address);
	else
		return run_host(tcp_address);

}