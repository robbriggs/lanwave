#include "LanWaveRPNToken.hpp"

using namespace std;
using namespace boost;

LanWaveRPNToken::LanWaveRPNToken(std::string token)
{
	if (token == "+")
		this->type = RPN_PLUS;
	else if (token == "-")
		this->type = RPN_MINUS;
	else if (token == "*")
		this->type = RPN_MUL;
	else if (token == "/")
		this->type = RPN_DIV;
	else if (token == "^")
		this->type = RPN_EXP;
	else if (token == "SIN")
		this->type = RPN_SIN;
	else if (token == "COS")
		this->type = RPN_COS;
	else if (token == "TAN")
		this->type = RPN_TAN;
	else if (token == "ASIN")
		this->type = RPN_ASIN;
	else if (token == "ACOS")
		this->type = RPN_ACOS;
	else if (token == "ATAN")
		this->type = RPN_ATAN;
	else if (token == "LOG")
		this->type = RPN_LOG;
	else if (token == "CHAN")
		this->type = RPN_CHAN;
	else if (token == "SLD")
		this->type = RPN_SLD;
	else if (token == "ID")
		this->type = RPN_ID;
	else if (token == "MAXID")
		this->type = RPN_MAXID;
	else if (token == "COUNT")
		this->type = RPN_COUNT;
	else if (token == "MOD")
		this->type = RPN_MOD;
	else if (token == "LAST")
		this->type = RPN_LAST;
	else if (token == "PI")
	{
		this->number = 3.14159265;
		this->type = RPN_NUM;
	}
	else
	{
		this->number = atof(token.c_str());
		this->type = RPN_NUM;
	}

	//cout << "RPNToken: this type is: " << this->type << endl;
}

int LanWaveRPNToken::get_type()
{
	return this->type;
}

double LanWaveRPNToken::get_number()
{
	return this->number;
}