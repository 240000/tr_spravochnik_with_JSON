#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>
#include <algorithm>
#include <deque>
#include <map>

#include "geo.h"


namespace transport_catalogue {
	
	struct Stop
	{
        std::string type_;
		std::string name_;
		geo::Coordinates coordinates_;
		std::set<std::string_view> buses_;
		std::unordered_map<std::string, int> distansies_to_stops_;
	};
	
	struct Bus
	{
        std::string type_;
		std::string name_;
		std::deque<std::string*> route_;
        bool is_roundtrip_;
	};
	
	class TransportCatalogue
	{
	public:
		void AddBus(std::string& type, std::string& bus_name, std::deque<std::string_view>& stops,  bool is_roundtrip);
		void AddStop(std::string& type, std::string& stop, geo::Coordinates& coordinates, std::unordered_map<std::string, int>& distansies_to_stops);
		std::set<std::string_view> SearchBusesByStop(std::string_view stop);
		Stop* IsStop(std::string_view stop);
		Bus* IsBus(std::string_view bus);
		std::tuple<int, double> ComputeAllDistance(const std::deque<std::string*>& route);
		std::tuple<int, int, int, double> RouteInfo(std::string_view bus);
	
	
	private:
		std::unordered_map<std::string*, const Bus*, std::hash<std::string*>> buses_;
		std::unordered_map<std::string*, const Stop*, std::hash<std::string*>> stops_;
		std::deque<Bus> buses_base_;
		std::deque<Stop> stops_base_;
	};
	
}//end transport_catalogue
