#pragma once

#include <string>
#include <iostream>
#include <vector>

namespace  Estelle{
	#define cref(Type) const Type&	
    #define FOR(i, size) for(int i=0;i < size;++i)
	typedef double Number;
	class Calculator {

		enum Operator { OP_Null, OP_Plus, OP_Minus, OP_Multi, OP_Div};
	public:
		inline bool IsNum(char c) {
			return (c >= '0' && c <= '9') || (c == '.');

		}
		inline Number ParseNum(cref(std::string) num) {
			Number ret=0;
			int i = 0;
			for (; i < num.size(); ++i) {
				auto& c = num[i];
				if (c == '.') break;
				int val = c - '0';
				ret *= 10.0;
				ret += val;
			}
			int pos = i;
			++i;
			for (; i < num.size(); ++i) {
				auto& c = num[i];
				if (c == '.') {
					throw std::runtime_error("Unexpected Token Period");
					
				}
				Number val = c - '0';
				val /= (std::pow(10, (i - pos)));
				ret += val;
			}
			return ret;
		}
		inline Number cal(Operator currOp, cref(std::string) num, cref(Number) currVal) {
			
			if (currOp == OP_Plus)
				return currVal+ParseNum(num);
			else if (currOp == OP_Minus)
				return currVal-ParseNum(num);
			else if (currOp == OP_Multi)
				return currVal * ParseNum(num);
			else 
				return currVal / ParseNum(num);
		}

		inline Operator ParseOP(const std::string& str, int& pos) {
			if (pos >= str.size()) {
				return OP_Null;
			}
			
			char c = str[pos];
			Operator currOp;
			switch (c) {
			case '+':
				currOp = OP_Plus;
				break;
			case '-':
				currOp = OP_Minus;
				break;

			case '*':
				currOp = OP_Multi;
				break;

			case '/':
				currOp = OP_Div;
				break;


			default:
				if (IsNum(str[pos])) return OP_Plus;
				throw std::runtime_error("Unrecognized Token");

			}

			++pos;
			return currOp;
		}

		inline Number ParseNum(const std::string& str, int& pos) {
			std::string currNum;
			for (; pos < str.size(); ++pos) {
				auto c = str[pos];
				if (IsNum(c)) {
					currNum += c;
				}
				else {
					return ParseNum(currNum);
				}
			}
			return ParseNum(currNum);
		}


		inline Number ApplyOP(Operator currOp, Number num, cref(Number) currVal) {

			if (currOp == OP_Plus)
				return currVal + num;
			else if (currOp == OP_Minus)
				return currVal - num;
			else if (currOp == OP_Multi)
				return currVal * num;
			else
				return currVal / num;

		}

		inline Number ParseRec(const std::string& str,int& pos, Operator OP, Number num, Operator& currentOP) {
			
			if (OP < currentOP) {
				Number currentNum = ParseNum(str, pos);
				Operator nextOP = ParseOP(str, pos);
				Number ret = ApplyOP(currentOP, ParseRec(str, pos, currentOP, currentNum, nextOP), num);
				currentOP = nextOP;
				while (OP < currentOP) {
					
					currentNum = ParseNum(str, pos);
					nextOP = ParseOP(str, pos);
					ret = ApplyOP(currentOP, ParseRec(str, pos, currentOP, currentNum, nextOP), ret);
					currentOP = nextOP;
				}
				currentOP = nextOP;
				return ret;
			}
			else {
				return num;
			}
		}

		inline Number ParseRec(const std::string& str, int pos) {
			
			Operator currentOP = ParseOP(str, pos);
			Number currentNum;
			Number result;
			Number ret=0;
			Operator nextOP;

			while (pos < str.size()) {
				currentNum = ParseNum(str, pos);
				nextOP = ParseOP(str, pos);
				result = ParseRec(str, pos, currentOP, currentNum, nextOP);
				ret = ApplyOP(currentOP, result, ret);
				
				currentOP = nextOP;
			}
			return ret;

		}

		inline Number Parse(const std::string& input) {
			return ParseRec(input, 0);
			Number ret=0;
			std::vector<Number> nums;
			std::string currNum;
			Operator currOp = OP_Plus;
			for (char c : input) {
				if (IsNum(c)) {
					currNum += c;
				}
				else {


					ret = cal(currOp, currNum, ret);
					currNum.clear();

					
				}
			}
			if (currNum.size()) {
				if (currOp == OP_Plus)
					ret += ParseNum(currNum);
				else
					ret -= ParseNum(currNum);
				currNum.clear();
			}

			return ret;
		}
		

	};
}