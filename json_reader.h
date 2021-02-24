#pragma once

#include "transport_catalogue.h"
#include "json.h"

#include <sstream>
#include <vector>
#include <string>
#include <string_view>
#include <iomanip>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {

    class Requests {
    public:
        void AddBaseRequests(const json::Dict & dictionary);
        void AddStatRequests(const json::Dict & dictionary);
        std::vector<json::Dict> GetBaseRequests() const;
        std::vector<json::Dict> GetStatRequests() const;
    private:
	
	    std::vector<json::Dict> base_requests_; // массив с описанием автобусных маршрутов и остановок.
	    std::vector<json::Dict> stat_requests_; // массив с запросами к транспортному справочнику.
    };

    void ParseRequests(Requests& requests, json::Document& document);

    json::Document LoadJSON(std::stringstream& input);

    void FillTransportCatologue(Requests& requests, transport_catalogue::TransportCatalogue& tc);
	
	void Output(const Requests& req, transport_catalogue::TransportCatalogue& tc, std::ostream& output);
	

}//end namespace json_reader



