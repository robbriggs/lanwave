#ifndef LANWAVERPN_H
#define LANWAVERPN_H

#include <iostream>
#include <string>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <stack>
#include <math.h>

#include "LanWaveRPNConstants.hpp"
#include "LanWaveRPNToken.hpp"
/*
struct visualiser_information {
	int frequency_values[8];
	int rid;
	int maxid;
	long long count;
	int slide1;
	int slide2;
};
*/
class LanWaveRPN {
	private:
		std::vector<LanWaveRPNToken*> tokens;
		//const visualiser_information* info;
		double last;

	public:
		//LanWaveRPN(std::string expression, const visualiser_information* inf);
		LanWaveRPN(std::string expression);
		~LanWaveRPN();
		float eval();
};

#endif