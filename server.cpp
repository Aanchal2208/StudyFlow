/* =====================================================================
   Lightweight HTTP server exposing the C++ algorithms to the frontend.
   Uses only standard sockets — no external libraries.
   Compile (Linux/macOS):
       g++ -std=c++17 server.cpp studyy.cpp expensee.cpp habitt.cpp \
           healthh.cpp reminderr.cpp roadmap.cpp auth.cpp -o scholaris_server
   Run:
       ./scholaris_server
   The frontend (index.html) talks to it on http://localhost:8080
   ===================================================================== */

#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "studyy.h"
#include "expensee.h"
#include "roadmap.h"
#include "auth.h"

using namespace std;

/* ----- minimal JSON helpers (no external deps). The frontend
   never sends complex nested JSON, so simple parsing suffices. */
static string extractField(const string& json, const string& key) {
    auto k = json.find("\"" + key + "\"");
    if (k == string::npos) return "";
    auto colon = json.find(':', k);
    auto start = json.find_first_not_of(" \t\"", colon+1);
    auto end = json.find_first_of(",}\"", start);
    return json.substr(start, end - start);
}

static string jsonResponse(const string& body) {
    ostringstream os;
    os << "HTTP/1.1 200 OK\r\n"
       << "Access-Control-Allow-Origin: *\r\n"
       << "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
       << "Access-Control-Allow-Headers: Content-Type\r\n"
       << "Content-Type: application/json\r\n"
       << "Content-Length: " << body.size() << "\r\n\r\n"
       << body;
    return os.str();
}

/* Route handler: /dijkstra { start, end } -> { path: [...], cost: int } */
static string handleDijkstra(const string& body) {
    string s = extractField(body, "start");
    string e = extractField(body, "end");
    auto graph = buildCurriculumGraph();
    auto res = dijkstra(graph, s, e);
    ostringstream os;
    os << "{\"path\":[";
    for (size_t i = 0; i < res.path.size(); ++i) {
        os << "\"" << res.path[i] << "\"";
        if (i+1 < res.path.size()) os << ",";
    }
    os << "],\"cost\":" << res.cost << "}";
    return os.str();
}

/* Route handler: /hash { input } -> { hash: "..." } */
static string handleHash(const string& body) {
    string in = extractField(body, "input");
    return "{\"hash\":\"" + sha256(in) + "\"}";
}

static void handleClient(int client) {
    char buf[8192] = {0};
    read(client, buf, sizeof(buf)-1);
    string req(buf);

    // Pre-flight CORS
    if (req.find("OPTIONS") == 0) {
        string r = "HTTP/1.1 204 No Content\r\n"
                   "Access-Control-Allow-Origin: *\r\n"
                   "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
                   "Access-Control-Allow-Headers: Content-Type\r\n\r\n";
        send(client, r.c_str(), r.size(), 0);
        close(client); return;
    }

    auto bodyStart = req.find("\r\n\r\n");
    string body = (bodyStart != string::npos) ? req.substr(bodyStart+4) : "";
    string out;

    if (req.find("POST /dijkstra") == 0)      out = handleDijkstra(body);
    else if (req.find("POST /hash") == 0)     out = handleHash(body);
    else                                      out = "{\"error\":\"unknown route\"}";

    string resp = jsonResponse(out);
    send(client, resp.c_str(), resp.size(), 0);
    close(client);
}

int main() {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    bind(srv, (sockaddr*)&addr, sizeof(addr));
    listen(srv, 16);
    cout << "Scholaris backend running on http://localhost:8080\n";

    while (true) {
        int client = accept(srv, nullptr, nullptr);
        if (client >= 0) thread(handleClient, client).detach();
    }
}
