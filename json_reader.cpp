#include "json_reader.h"

using namespace std;

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {

	void Requests::AddBaseRequests(const json::Dict & dictionary) {
		base_requests_.push_back(dictionary);
	}
	void Requests::AddStatRequests(const json::Dict & dictionary) {
		stat_requests_.push_back(dictionary);
	}
	std::vector<json::Dict> Requests::GetBaseRequests() const {
		return base_requests_;
	}
	std::vector<json::Dict> Requests::GetStatRequests() const {
		return stat_requests_;
	}



	void ParseRequests(Requests& requests, json::Document& document) {
		//if(document.GetRoot().IsMap()) {
			auto map = document.GetRoot().AsMap();

			if(map["base_requests"].IsArray()) {
				json::Array array = map["base_requests"].AsArray();
				for(auto & a : array)
					requests.AddBaseRequests(a.AsMap());
			}
			if(map["stat_requests"].IsArray()) {
				json::Array array = map["stat_requests"].AsArray();
				for(const auto& a : array) {
					requests.AddStatRequests(a.AsMap());
				}
			}
		//}
	}

	json::Document LoadJSON(std::stringstream& input) {
		return json::Load(input);
	}

	void FillTransportCatologue(Requests& requests, transport_catalogue::TransportCatalogue& tc) {
		for(json::Dict& stop : requests.GetBaseRequests()) {
			if (stop["type"].AsString() == "Stop") {
			    std::string type = stop["type"].AsString();
				std::string name  = stop["name"].AsString();
				geo::Coordinates coordinates = { stop["latitude"].AsDouble(), stop["longitude"].AsDouble() };
				std::unordered_map<std::string, int> road_distances;
				for(auto & [key, value] : stop["road_distances"].AsMap()) {
					road_distances.insert({key, value.AsInt() });
				}
				tc.AddStop(type, name, coordinates,road_distances);
			}
		}
		for (json::Dict& bus : requests.GetBaseRequests()) {
			if (bus["type"].AsString() == "Bus") {
                std::string type = bus["type"].AsString();
				std::string name  = bus["name"].AsString();
				bool is_roundtrip = bus["is_roundtrip"].AsBool();
				std::deque<std::string_view> stops;
				for(const auto& s : bus["stops"].AsArray()) {
				    std::string temp = s.AsString();
				    if(const transport_catalogue::Stop* stop = tc.IsStop(temp)) {
                        stops.push_back(stop->name_);
                    }
				}
				if(!is_roundtrip && !stops.empty()) {
                    std::deque<std::string_view> stops_copy = stops;
                    stops.pop_back();
                    for(auto it = stops_copy.rbegin(); it != stops_copy.rend(); ++it) {
                        stops.push_back(*it);
                    }
				}
				tc.AddBus(type, name, stops, is_roundtrip);
			}
		}
	}

	void Output(const Requests& req, transport_catalogue::TransportCatalogue& tc, std::ostream& output) {
		output << "[" << std::endl;
		bool not_first_element = false;
		for (auto &request : req.GetStatRequests()) {
			if(not_first_element) {
				output << "," << std::endl;
			}
			not_first_element = true;
			output << " " << "{" << std::endl;
			if (request["type"].AsString() == "Bus") {
				if (tc.IsBus(request["name"].AsString())) {
					std::tuple<int, int, int, double> result = tc.RouteInfo(request["name"].AsString());
					output << "  " << "\"curvature\": " << get<3>(result) << "," << std::endl
						 << "  " << "\"request_id\": " << request["id"].AsInt() << "," << std::endl
						 << "  " << "\"route_length\": " << get<2>(result) << "," << std::endl
						 << "  " << "\"stop_count\": " << get<0>(result) << "," << std::endl
						 << "  " << "\"unique_stop_count\": " << get<1>(result) << std::endl;
				}
				else {
					output << "  " << "\"request_id\": " << request["id"].AsInt() << "," << std::endl
					     << "  " << "\"error_message\": \"not found\"" << std::endl;
				}
			}
			if (request["type"].AsString() == "Stop") {
				if (tc.IsStop(request["name"].AsString())) {
					std::set<std::string_view> buses = tc.SearchBusesByStop(request["name"].AsString());
					output << "  " << "\"buses\": [";
					if(!buses.empty()) {
						for (std::string_view bus : buses) {
							if (bus == *buses.begin()) {
								output << std::endl << "   " << "\"" << bus << "\"";
							}
							else {
								output << ", " << "\"" << bus << "\"";
							}
						}
						output << std::endl << "  " << "]," << std::endl;
					}
					else {
						output << " ]," << std::endl;
					}
					output << "  " << "\"request_id\": " << request["id"].AsInt() << std::endl;
				}
				else {
					output << "  " << "\"request_id\": " << request["id"].AsInt() << "," << std::endl
						 << "  " << "\"error_message\": \"not found\"" << std::endl;
				}
			}
			output << " " << "}";
		}
		output << std::endl << "]";
	}


} // end namespace json_reader
