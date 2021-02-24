#include <sstream>
#include <iomanip>
#include <utility>
#include "json.h"

using namespace std;

namespace json {
	
	bool operator== (const Node& lhs, const Node& rhs) {
		return lhs.GetVariant() == rhs.GetVariant();
	}
	bool operator!= (const Node& lhs, const Node& rhs) {
		return !(lhs.GetVariant() == rhs.GetVariant());
	}
	bool operator== (const Document& lhs, const Document& rhs) {
		return lhs.GetRoot() == rhs.GetRoot();
	}
	bool operator!= (const Document& lhs, const Document& rhs) {
		return !(lhs.GetRoot() == rhs.GetRoot());
	}
	
	
//------------ Load -------------


	namespace {
	
	using Number = std::variant<int, double>;
	
	Node LoadNode(stringstream& input);
	
	Number LoadNumber(std::stringstream& input) {
		using namespace std::literals;
		
		std::string parsed_num;
		
		// Считывает в parsed_num очередной символ из input
		auto read_char = [&parsed_num, &input] {
			parsed_num += static_cast<char>(input.get());
			if (!input) {
				throw ParsingError("Failed to read number from stream"s);
			}
		};
		
		// Считывает одну или более цифр в parsed_num из input
		auto read_digits = [&input, read_char] {
			if (!std::isdigit(input.peek())) {
				throw ParsingError("A digit is expected"s);
			}
			while (std::isdigit(input.peek())) {
				read_char();
			}
		};
		
		if (input.peek() == '-') {
			read_char();
		}
		// Парсим целую часть числа
		if (input.peek() == '0') {
			read_char();
			// После 0 в JSON не могут идти другие цифры
		} else {
			read_digits();
		}
		
		bool is_int = true;
		// Парсим дробную часть числа
		if (input.peek() == '.') {
			read_char();
			read_digits();
			is_int = false;
		}
		
		// Парсим экспоненциальную часть числа
		if (int ch = input.peek(); ch == 'e' || ch == 'E') {
			read_char();
			if (ch = input.peek(); ch == '+' || ch == '-') {
				read_char();
			}
			read_digits();
			is_int = false;
		}
		
		try {
			if (is_int) {
				// Сначала пробуем преобразовать строку в int
				try {
					return std::stoi(parsed_num);
				} catch (...) {
					// В случае неудачи, например, при переполнении
					// код ниже попробует преобразовать строку в double
				}
			}
			return std::stod(parsed_num);
		} catch (...) {
			throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
		}
	}
	
	Node LoadArray(stringstream& input) {
		Array result;
		
		for (char c; input >> c && c != ']';) {
			if (c != ',') {
				input.putback(c);
			}
			result.push_back(LoadNode(input));
		}
		
		return Node(move(result));
	}
	
	Node LoadString(stringstream& input) {
		string res;
		int quot_count = 0;
		if(input.peek() == '"') {
			char quot;
			input >> quot;
		}
		for (char c; input >> c && c != '"';) {
			if (c == '\\' && input.peek() == '\\') {
				res .push_back('\\');
				input.ignore(1);
			}
			else if (c == '\\' && input.peek() == '"') {
				res .push_back('\"');
				input.ignore(1);
			}
			else if(c == '\\' && input.peek() == 'n') {
				res.push_back('\n');
				input.ignore(1);
			}
			else if(c == '\\' && input.peek() == 'r') {
				res.push_back('\r');
				input.ignore(1);
			}
			else if(c == '\\' && input.peek() == 't') {
				res.push_back('\t');
				input.ignore(1);
			}
			else if((c == '"' && input.peek() == ',')
					|| (c == '"' && input.peek() == '}')
					|| (c == '"' && input.peek() == ']')) {
				if (c == '"') {
					++quot_count;
				}
				break;
			}
			else if (c == '"') {
				++quot_count;
			}
			else if(c == '\\' || c == '\"' || c == '\n' || c == '\r' || c == '\t') {}
			else if((c == ' ' && input.peek() == ']')
					|| (c == ' ' && input.peek() == '}')) {
				break;
			}
			else if(c == ':') {
				
				input.putback(c);
				break;
			}
			else {
				res.push_back(c);
			}
		}
		if(quot_count % 2 != 0) {
			throw ParsingError("quotation marks are open"s);
		}
		return Node(move(res));
	}
	
	Node LoadDict(stringstream& input) {
		Dict result;
		
		for (char c; input >> c && c != '}';) {
			if (c == ',') {
				input >> c;
			}
			input.putback(c);
			string key = LoadString(input).AsString();
			result.insert({move(key), LoadNode(input)});
		}
		
		return Node(move(result));
	}
	
	Node LoadNode(stringstream& input) {
		char c;
		input >> c;
		if(c == ':') {
			input >> c;
		}
		if (c == '[') {
			char c2;
			if(input >> c2) {
				input.putback(c2);
				return LoadArray(input);
			}
			throw ParsingError("invalid array"s);
		}
		else if (c == '{') {
			char c2;
			if(input >> c2) {
				input.putback(c2);
				return LoadDict(input);
			}
			throw ParsingError("invalid dictionary"s);
		}
		else if(c == '"') {
			input.putback(c);
			return LoadString(input);
		}
		else if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
			input.putback(c);
			string s;
			input >> s;
			if(s.back() == ',' || s.back() == '}' || s.back() == ']') {
				input.putback(s.back());
				s.erase(s.size() - 1, 1);
			}
			if(s == "null" || s == "\"null\"") {
				return Node(nullptr);
			}
			else if (s == "true" || s == "false" || s == "\"true\"" || s == "\"false\"") {
				bool b = true;
				if (s == "false" || s == "\"false\"") {
					b = false;
				}
				return Node(b);
			}
			else if (s == "tru" || s == "fals" || s == "nul") {
				throw ParsingError("try again"s);
			}
			else {
				for (auto it = s.end(); it != s.begin(); ++it) {
					input.putback(*it);
				}
				return LoadString(input);
			}
		}
		else {
			input.putback(c);
			const auto res = LoadNumber(input);
			if(holds_alternative<int>(res)) {
				return get<int>(res);
			}
			else {
				return get<double>(res);
			}
		}
	}
	
	}  // namespace
	

//------------ Node -------------

	
	Node::Node()
			: node_(nullptr) {
	}
	Node::Node(nullptr_t)
			: node_(nullptr) {
	}
	Node::Node(bool value)
			: node_(value) {
	}
	Node::Node(int value)
			: node_(value) {
	}
	Node::Node(double value)
			: node_(value) {
	}
	Node::Node(string value)
			: node_(move(value)) {
	}
	Node::Node(Array array)
			: node_(move(array)) {
	}
	Node::Node(Dict map)
			: node_(move(map)) {
	}
	Node::Node(Variant v)
			: node_(move(v)) {
	}
	
	bool Node::AsBool() const {
		if(holds_alternative<bool>(node_)) {
			return *get_if<bool>(&node_);
		}
		throw logic_error("expected bool"s);
	}
	int Node::AsInt() const {
		if(holds_alternative<int>(node_)) {
			return *get_if<int>(&node_);
		}
		throw logic_error("expected int"s);
	}
	double Node::AsDouble() const {
		if(holds_alternative<int>(node_) || holds_alternative<double>(node_)) {
			if(IsInt()) {
				return static_cast<double>(AsInt());
			}
			return *get_if<double>(&node_);
		}
		throw logic_error("expected int or double"s);
	}
    double Node::AsPureDouble() const {
        if(holds_alternative<double>(node_)) {
            return *get_if<double>(&node_);
        }
        throw logic_error("expected pure double"s);
    }
	string Node::AsString() const {
		if(holds_alternative<string>(node_)) {
			return *get_if<string>(&node_);
		}
		throw logic_error("expected string"s);
	}
	Array Node::AsArray() const {
		if(holds_alternative<Array>(node_)) {
			return *get_if<Array>(&node_);
		}
		throw logic_error("expected array"s);
	}
	Dict Node::AsMap() const {
		if(holds_alternative<Dict>(node_)) {
			return *get_if<Dict>(&node_);
		}
		throw logic_error("expected dictionary"s);
	}
	
	bool Node::IsNull() const { return holds_alternative<nullptr_t>(node_); }
	bool Node::IsBool() const { return holds_alternative<bool>(node_);}
	bool Node::IsInt() const { return holds_alternative<int>(node_);}
	bool Node::IsDouble() const { return holds_alternative<double >(node_) || holds_alternative<int>(node_);}
	bool Node::IsPureDouble() const { return holds_alternative<double>(node_);}
	bool Node::IsString() const { return holds_alternative<string>(node_);}
	bool Node::IsArray() const { return holds_alternative<Array>(node_);}
	bool Node::IsMap() const { return holds_alternative<Dict>(node_);}
	
	const Variant& Node::GetVariant() const {
		return node_;
	}
	
	
//------------ Document -------------

	
	Document::Document(Node root)
			: root_(std::move(root)) {
	}
	
	Node Document::GetRoot() const {
		return root_;
	}
	
	Document Load(stringstream& input) {
		return Document{LoadNode(input)};
	}
	
	void Print(const Document& doc, std::ostream& output) {
		if(doc.GetRoot().IsNull()) {
			output << "null"s;
		}
		else if(doc.GetRoot().IsBool()) {
			if(doc.GetRoot().AsBool()) {
				output << "true";
			}
			else {
				output << "false";
			}
		}
		else if(doc.GetRoot().IsInt()) {
			output << doc.GetRoot().AsInt();
		}
		else if(doc.GetRoot().IsDouble()) {
			output << doc.GetRoot().AsDouble();
		}
		else if(doc.GetRoot().IsString()) {
			string res;
			res += "\""s;
			for(const char& c : doc.GetRoot().AsString()) {
				if (c == '\\') {
					res += R"(\\)";
				}
				else if (c == '\"') {
					res += R"(\")";
				}
				else if (c == '\n') {
					res += R"(\n)";
				}
				else if (c == '\r') {
					res += R"(\r)";
				}
				else if (c == '\t') {
					res += R"(\t)";
				}
				else {
					res += c;
				}
			}
			res += "\""s;
			output << res;
		}
		else if(doc.GetRoot().IsArray()) {
			auto arr = doc.GetRoot().AsArray();
			output << '[';
			for(size_t i = 0; i < arr.size(); ++i) {
				if(i == 0) {
					Print({ arr[i] }, output);
				}
				else {
					output << ", ";
					Print({ arr[i] }, output);
				}
			}
			output << "]";
		}
		else if(doc.GetRoot().IsMap()) {
			auto map = doc.GetRoot().AsMap();
			output << "{"s;
			for(const auto& [key, value] : map) {
				if(map.find(key) == map.begin()) {
					Print({ key }, output);
					output << ": "s;
					Print({ value }, output);
				}
				else {
					output << ", ";
					Print({ key }, output);
					output << ": "s;
					Print({ value }, output);
				}
			}
			output << "}";
		}
		
		// Реализуйте функцию самостоятельно
	}
	
}  // namespace json
