#ifndef LANWAVERPNTOKEN_H
#define LANWAVERPNTOKEN_H

#include <iostream>
#include <string>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <stack>
#include <math.h>

#include "LanWaveRPNConstants.hpp"

class LanWaveRPNToken {
	private:
		double number;
		int type;

	public:
		LanWaveRPNToken(std::string token);
		int get_type();
		double get_number();
};

#endif