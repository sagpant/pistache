/* 
   Mathieu Stefani, 07 février 2016
   
 * Http client example
*/

#include "net.h"
#include "http.h"
#include "client.h"

using namespace Net;
using namespace Net::Http;

int main(int argc, char *argv[]) {
    if (argc < 1) {
        std::cerr << "Usage: http_client page [count]" << std::endl;
        return 1;
    }

    std::string page = argv[1];
    int count = 1;
    if (argc == 3) {
        count = std::stoi(argv[2]);
    }

    Experimental::Client client;

    auto opts = Http::Experimental::Client::options()
        .threads(1)
        .maxConnectionsPerHost(8);
    client.init(opts);

    std::vector<Async::Promise<Response>> responses;

    std::atomic<size_t> completedRequests(0);
    std::atomic<size_t> failedRequests(0);

    auto start = std::chrono::system_clock::now();

    for (int i = 0; i < count; ++i) {
        auto resp = client.get(page).cookie(Cookie("FOO", "bar")).send();
        resp.then([&](Response response) {
                ++completedRequests;
            std::cout << "Response code = " << response.code() << std::endl;
            auto body = response.body();
            if (!body.empty())
               std::cout << "Response body = " << body << std::endl;
        }, Async::IgnoreException);
        responses.push_back(std::move(resp));
    }

    auto sync = Async::whenAll(responses.begin(), responses.end());
    Async::Barrier<std::vector<Response>> barrier(sync);

    barrier.wait_for(std::chrono::seconds(5));

    auto end = std::chrono::system_clock::now();
    std::cout << "Summary of excution" << std::endl
              << "Total number of requests sent     : " << count << std::endl
              << "Total number of responses received: " << completedRequests.load() << std::endl
              << "Total number of requests failed   : " << failedRequests.load() << std::endl
              << "Total time of execution           : "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    client.shutdown();
}
