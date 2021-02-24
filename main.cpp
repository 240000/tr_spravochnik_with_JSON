#include "json_reader.h"
#include "transport_catalogue.h"
#include "json.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <chrono>

using namespace std;


void FileInput(const std::string& filename, std::stringstream& input) {
	
	std::string s;
	std::string temp;
	std::ifstream in(filename);
	
	
	if (in.is_open())
	{
		for(std::string s; getline(in, s) && (*prev(input.str().end()) != ']' || input.str().back() != '}');) {
			input << s;
		}
	}
	in.close();     // закрываем файл
	
	
}

void FileOutput(json_reader::Requests& req, transport_catalogue::TransportCatalogue& tc, const std::string& filename) {
	
	std::ofstream out(filename);
	
	if (out.is_open())
	{
		json_reader::Output(req, tc, out);
	}
	out.close();     // закрываем файл
}



int main()
{
	
	std::string in_filename = "/Users/Evgenij/Desktop/С++/Я.Практикум/Элегантность и массштабируемость/Наследование и полиморфизм/Транспортный справочник — поддержка JSON/На отправку куратору/input1.json";
	std::stringstream input;
	FileInput(in_filename, input);
	json_reader::Requests requests;
	json::Document document = json_reader::LoadJSON(input);
	json_reader::ParseRequests(requests, document);
	transport_catalogue::TransportCatalogue tc;
	json_reader::FillTransportCatologue(requests, tc);
	string out_filename = "/Users/Evgenij/Desktop/С++/Я.Практикум/Элегантность и массштабируемость/Наследование и полиморфизм/Транспортный справочник — поддержка JSON/На отправку куратору/output1.json";
	std::ofstream out(out_filename);
	FileOutput(requests, tc, out_filename);
	
	return 0;
}

