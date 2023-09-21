#include "shard_app.h"
///
/// \brief main
/// \param argc
/// \param argv
/// \return
///
int main(int argc, const char* argv[]) {
	auto server = shard_app::createShardApp(argc, argv);
	return shard_app::run(server);
}




