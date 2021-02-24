#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

//------------ Node -------------


	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	using Variant = std::variant<std::nullptr_t, bool, int, double, std::string, Array, Dict>;
	
	class Node {
	public:
		Node();
		Node(std::nullptr_t);
		Node(bool value);
		Node(int value);
		Node(double value);
		Node(std::string value);
		Node(Array array);
		Node(Dict map);
		Node(Variant v);
		
		bool AsBool() const;
		int AsInt() const;
		double AsDouble() const;
        double AsPureDouble() const;
		std::string AsString() const;
		Array AsArray() const;
		Dict AsMap() const;
		
		bool IsNull()const;
		bool IsBool()const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsString() const;
		bool IsArray() const;
		bool IsMap() const;
		
	const Variant& GetVariant() const;
	
	private:
		Variant node_;
	};
	
	bool operator== (const Node& lhs, const Node& rhs);
	bool operator!= (const Node& lhs, const Node& rhs);


//------------ Document -------------

	
	class Document {
	public:
		Document() = default;
		Document(Node root);
		
		Node GetRoot() const;
	
	private:
		Node root_;
	};
	
	bool operator== (const Document& lhs, const Document& rhs);
	bool operator!= (const Document& lhs, const Document& rhs);
	
	Document Load(std::stringstream& input);
	
	void Print(const Document& doc, std::ostream& output);
	
}  // namespace json
