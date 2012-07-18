#include "LanWaveRPN.hpp"
#include "global.hpp"

using namespace std;
using namespace boost;

//LanWaveRPN::LanWaveRPN(std::string str, const visualiser_information *inf)
LanWaveRPN::LanWaveRPN(std::string str)
{
    //std::cout << "LanWaveRPN Constructor: infoPackage address received: " << inf << endl;

	typedef boost::tokenizer<boost::char_separator<char> > 
    tokenizer;
  	boost::char_separator<char> sep(" ", "\n");
  	tokenizer tokens(str, sep);
    //this->info = inf;

    //std::cout << "LanWaveRPN Constructor: infoPackage address assigned: " << this->info << endl;

  	if (tokens.begin() == tokens.end())
  		cout << "Error, rpn is parsing an empty string\n";
  	else
  	{
	  	for (tokenizer::iterator tok_iter = tokens.begin();
	       tok_iter != tokens.end(); ++tok_iter)
	  	{
            if ( tok_iter->substr(0,1) == ":" )
            {
                this->last = atof(tok_iter->c_str()+1);
                return;
            }else
	  		   this->tokens.push_back(new LanWaveRPNToken(*tok_iter));
	  	}
  	}
}

LanWaveRPN::~LanWaveRPN()
{
	for(std::vector<LanWaveRPNToken>::size_type i = 0; i != this->tokens.size(); i++) {
		delete tokens[i];
	}
}

float LanWaveRPN::eval()
{
	stack<double> stack;
	double op1, op2, result;
	int intop1, intop2, intresult;

	for(std::vector<LanWaveRPNToken>::size_type i = 0; i != this->tokens.size(); i++) {
    	if (tokens[i]->get_type() == RPN_NUM)
    		stack.push(tokens[i]->get_number());
    	else if ( tokens[i]->get_type() < PRN_TWOOPS )
    	{
    		// Two operands
    		if ( stack.size() < 2 )
    		{
    			cout << "Error: token " << i << " should pop two off the stack, but there isn't two to pop.\n";
    			return 0;
    		}

    		op1 = stack.top();
    		stack.pop();
    		op2 = stack.top();
    		stack.pop();

    		if (tokens[i]->get_type() == RPN_PLUS)
    			result = op2 + op1;
    		else if (tokens[i]->get_type() == RPN_MINUS)
    			result = op1 - op2;
    		else if (tokens[i]->get_type() == RPN_MUL)
    			result = op1 * op2;
    		else if (tokens[i]->get_type() == RPN_DIV)
    			result = op2 / op1;
            else if (tokens[i]->get_type() == RPN_EQUAL)
                result = (op1 == op2) ? 1 : 0;
            else if (tokens[i]->get_type() == RPN_GT)
                result = (op1 < op2) ? 1 : 0;
            else if (tokens[i]->get_type() == RPN_END)
            {
                cout << "op1 = " << op1 << ", op2 = " << op2 << endl;
                if (op2 == 0)
                    return op1;
            }
    		else if (tokens[i]->get_type() == RPN_MOD)
    		{
    			intop1 = op1;
    			intop2 = op2;
    			cout << intop2 << " % " << intop1 << endl;
    			intresult = intop2 % intop1;
				result = intresult;
			}
			else if (tokens[i]->get_type() == RPN_EXP)
				result = pow(op2, op1);
    		else
    			cout << "Parsing error, did not understand token " << tokens[i]->get_type() << " in two operands eval.\n";

    		stack.push(result);
    	}
    	else if ( tokens[i]->get_type() < RPN_ONEOP )
    	{
    		// One operand
    		if ( stack.size() < 1 )
    		{
    			cout << "Error: token " << i << " should pop one off the stack, but there isn't one to pop.\n";
    			return 0;
    		}

    		op1 = stack.top();
    		stack.pop();

			if (tokens[i]->get_type() == RPN_SIN)
    			result = sin( op1 );
    		else if (tokens[i]->get_type() == RPN_COS)
    			result = cos( op1 );
    		else if (tokens[i]->get_type() == RPN_TAN)
    			result = tan( op1 );
    		else if (tokens[i]->get_type() == RPN_ASIN)
    			result = asin( op1 );
    		else if (tokens[i]->get_type() == RPN_ACOS)
    			result = acos( op1 );
    		else if (tokens[i]->get_type() == RPN_ATAN)
    			result = atan( op1 );
    		else if (tokens[i]->get_type() == RPN_LOG)
    			result = log( op1 );
            else if (tokens[i]->get_type() == RPN_SLD)
            {
                intop1 = op1;
                result = infoPackage->slide[intop1];
            }
    		else if (tokens[i]->get_type() == RPN_CHAN)
    		{
    			intop1 = op1;
    			result = infoPackage->frequency_values[intop1%8];
                //cout << "RPN: Got " << result << " from chan " << intop1%8 << endl;
    		}
    		else
    			cout << "Parsing error, did not understand token " << tokens[i]->get_type() << " in one operands eval.\n";

    		stack.push(result);
    	}
    	else
    	{
    		// No Operands
    		if (tokens[i]->get_type() == RPN_ID)
    			stack.push(infoPackage->rid);
    		else if (tokens[i]->get_type() == RPN_MAXID)
    			stack.push(infoPackage->maxid);
    		else if (tokens[i]->get_type() == RPN_COUNT)
    			stack.push(infoPackage->count);
            else if (tokens[i]->get_type() == RPN_LAST)
                stack.push(this->last);
			else
    			cout << "Parsing error, did not understand token " << tokens[i]->get_type() << " in no operands eval.\n";
    	}
	}

    this->last = stack.top();

	return (float)stack.top();
}
