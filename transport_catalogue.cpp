#include <iostream>
#include <iomanip>
#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue {

    void TransportCatalogue::AddBus(std::string& type, std::string& bus_name, std::deque<std::string_view>& stops,  bool is_roundtrip) {
		buses_base_.push_front({ move(type), move(bus_name), {}, is_roundtrip });
		for (string_view stop : stops) {
			Stop *ptr = IsStop(stop);
			if (ptr != nullptr) {
				buses_base_.front().route_.emplace_front(&ptr->name_);
				ptr->buses_.emplace(buses_base_.front().name_);
			}
		}
		buses_[&buses_base_.front().name_] = &buses_base_.front();
	}

    void TransportCatalogue::AddStop(std::string& type, std::string& stop_name, geo::Coordinates& coordinates,
                                                std::unordered_map<std::string, int>& distansies_to_stops)
	{
		stops_base_.push_front({ move(type), move(stop_name), coordinates, {}, move(distansies_to_stops) });
		stops_[&stops_base_.front().name_] = &stops_base_.front();
	}
	
	set<string_view> TransportCatalogue::SearchBusesByStop(string_view stop)
	{
		Stop *ptr = IsStop(stop);
		return stops_[&ptr->name_]->buses_;
	}
	
	Stop *TransportCatalogue::IsStop(string_view stop)
	{
		auto it = find_if(stops_base_.begin(), stops_base_.end(), [&](Stop &s) { return s.name_ == stop; });
		if (it != stops_base_.end()) {
			return &stops_base_[static_cast<int>(it - stops_base_.begin())];
		}
		return nullptr;
	}
	
	Bus *TransportCatalogue::IsBus(string_view bus)
	{
		auto it = find_if(buses_base_.begin(), buses_base_.end(), [&](Bus &b) { return b.name_ == bus; });
		if (it != buses_base_.end()) {
			return &buses_base_[static_cast<int>(it - buses_base_.begin())];
		}
		return nullptr;
	}
	
	tuple<int, double> TransportCatalogue::ComputeAllDistance(const deque<string*>& route)
	{
		int real_route_length = 0.0;
		double geo_route_length = 0.0;
		if(!route.empty()) {
			for (auto it = next(route.begin()); it != route.end(); ++it) {
				if (stops_[*prev(it)]->distansies_to_stops_.count(stops_[*it]->name_) != 0) {
					real_route_length += stops_[*prev(it)]->distansies_to_stops_.at(stops_[*it]->name_);
				}
				else {
					real_route_length += stops_[*it]->distansies_to_stops_.at(stops_[*prev(it)]->name_);
				}
				if (route.size() > 1) {
					geo_route_length += ComputeDistance(stops_[*prev(it)]->coordinates_, stops_[*it]->coordinates_);
				}
			}
		}
		return {real_route_length, geo_route_length};
	}
	
	tuple<int, int, int, double> TransportCatalogue::RouteInfo(string_view bus)
	{
		tuple<int, int, int, double> result;
		Bus *bus_ptr = IsBus(bus);
		int stops_on_route = buses_[&bus_ptr->name_]->route_.size();
		set<string *> temp{buses_[&bus_ptr->name_]->route_.begin(), buses_[&bus_ptr->name_]->route_.end()};
		int unique_stops = temp.size();
		tuple<int, double> all_distancies = ComputeAllDistance(buses_[&bus_ptr->name_]->route_);
		int real_route_length = get<0>(all_distancies);
		double geo_route_length = get<1>(all_distancies);
		double curvature = real_route_length / geo_route_length;
		result = {stops_on_route, unique_stops, real_route_length, curvature};
		return result;
	}
	
}//end transport_catalogue
