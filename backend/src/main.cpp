#include "compare.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

namespace {

using json = nlohmann::json;

std::string environmentValue(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return value == nullptr || *value == '\0' ? fallback : value;
}

int configuredPort() {
    const std::string value = environmentValue("PORT", "8080");
    try {
        const int port = std::stoi(value);
        if (port >= 1 && port <= 65535) {
            return port;
        }
    } catch (const std::exception&) {
    }

    std::cerr << "Invalid PORT value '" << value << "'; using 8080 instead.\n";
    return 8080;
}

void addCorsHeaders(httplib::Response& response, const std::string& origin) {
    response.set_header("Access-Control-Allow-Origin", origin);
    response.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    response.set_header("Access-Control-Allow-Headers", "Content-Type");
    response.set_header("Vary", "Origin");
}

void sendJson(httplib::Response& response, const json& body, int status = 200) {
    response.status = status;
    response.set_content(body.dump(), "application/json; charset=utf-8");
}

json lineRangesToJson(const std::vector<codeplag::LineRange>& ranges) {
    json output = json::array();
    for (const auto& [start, end] : ranges) {
        output.push_back({start, end});
    }
    return output;
}

}  // namespace

int main() {
    const int port = configuredPort();
    const std::string frontendOrigin =
        environmentValue("FRONTEND_ORIGIN", "http://localhost:3000");

    httplib::Server server;

    server.Get("/health", [](const httplib::Request&, httplib::Response& response) {
        sendJson(response, {{"status", "ok"}});
    });

    server.Options(
        "/compare",
        [&frontendOrigin](const httplib::Request&, httplib::Response& response) {
            addCorsHeaders(response, frontendOrigin);
            response.status = 204;
        });

    server.Post(
        "/compare",
        [&frontendOrigin](const httplib::Request& request, httplib::Response& response) {
            addCorsHeaders(response, frontendOrigin);

            try {
                const json body = json::parse(request.body);
                if (!body.is_object() || !body.contains("code1") ||
                    !body["code1"].is_string() || !body.contains("code2") ||
                    !body["code2"].is_string()) {
                    sendJson(
                        response,
                        {{"error", "code1 and code2 are required and must be strings"}},
                        400);
                    return;
                }

                const auto result = codeplag::compareCode(
                    body["code1"].get<std::string>(),
                    body["code2"].get<std::string>());

                sendJson(response, {
                    {"similarity", result.similarity},
                    {"matchedLines1", lineRangesToJson(result.matchedLines1)},
                    {"matchedLines2", lineRangesToJson(result.matchedLines2)},
                    {"totalFingerprints1", result.totalFingerprints1},
                    {"totalFingerprints2", result.totalFingerprints2},
                    {"matchedFingerprints", result.matchedFingerprints},
                });
            } catch (const json::parse_error&) {
                sendJson(response, {{"error", "request body must be valid JSON"}}, 400);
            } catch (const std::exception& error) {
                std::cerr << "Comparison failed: " << error.what() << '\n';
                sendJson(response, {{"error", "comparison failed"}}, 500);
            }
        });

    std::cout << "codeplag backend listening on http://0.0.0.0:" << port << '\n';
    if (!server.listen("0.0.0.0", port)) {
        std::cerr << "Unable to listen on port " << port << '\n';
        return 1;
    }

    return 0;
}
