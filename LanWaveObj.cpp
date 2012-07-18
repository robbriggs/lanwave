#include <iostream>
#include <string>

#include "LanWaveObj.hpp"
#include "LanWaveRPN.hpp"

LanWaveObj::LanWaveObj(std::string modelName)
{
	model_name = modelName;
}

LanWaveObj::~LanWaveObj()
{
		delete posX_RPN;
		delete posY_RPN;
		delete posZ_RPN;

		delete sclX_RPN;
		delete sclY_RPN;
		delete sclZ_RPN;

		delete rotX_RPN;
		delete rotY_RPN;
		delete rotZ_RPN;

		delete colR_RPN;
		delete colG_RPN;
		delete colB_RPN;
}

std::string LanWaveObj::getModelName()
{
	return model_name;
}
/*
void LanWaveObj::setInfoPackage(const visualiser_information *infoPackage)
{
	//std::cout << "LanWaveObj: address received is " << infoPackage << std::endl;
	const visualiser_information ptr_infoPackage = infoPackage;
	//std::cout << "LanWaveObj: address assigned is " << ptr_infoPackage << std::endl;
}
*/
void LanWaveObj::setModelID(int id)
{
	modelID = id;
}

void LanWaveObj::givePosX(std::string input)
{
	posX_RPN = new LanWaveRPN(input);
}

void LanWaveObj::givePosY(std::string input)
{
	posY_RPN = new LanWaveRPN(input);
}

void LanWaveObj::givePosZ(std::string input)
{
	posZ_RPN = new LanWaveRPN(input);
}

void LanWaveObj::giveSclX(std::string input)
{
	sclX_RPN = new LanWaveRPN(input);
}

void LanWaveObj::giveSclY(std::string input)
{
	sclY_RPN = new LanWaveRPN(input);
}

void LanWaveObj::giveSclZ(std::string input)
{
	sclZ_RPN = new LanWaveRPN(input);
}

void LanWaveObj::giveRotX(std::string input)
{
	rotX_RPN = new LanWaveRPN(input);
}

void LanWaveObj::giveRotY(std::string input)
{
	rotY_RPN = new LanWaveRPN(input);
}

void LanWaveObj::giveRotZ(std::string input)
{
	rotZ_RPN = new LanWaveRPN(input);
}

void LanWaveObj::giveColR(std::string input)
{
	colR_RPN = new LanWaveRPN(input);
}

void LanWaveObj::giveColG(std::string input)
{
	colG_RPN = new LanWaveRPN(input);
}

void LanWaveObj::giveColB(std::string input)
{
	colB_RPN = new LanWaveRPN(input);
}

void LanWaveObj::setWireframe(bool state)
{
	wireframe = state;
}

float LanWaveObj::getPosX()
{

	return posX_RPN->eval();
}

float LanWaveObj::getPosY()
{
	return posY_RPN->eval();
}

float LanWaveObj::getPosZ()
{
	return posZ_RPN->eval();
}

float LanWaveObj::getSclX()
{
	return sclX_RPN->eval();
}

float LanWaveObj::getSclY()
{
	return sclY_RPN->eval();
}

float LanWaveObj::getSclZ()
{
	return sclZ_RPN->eval();
}

float LanWaveObj::getRotX()
{
	return rotX_RPN->eval();
}

float LanWaveObj::getRotY()
{
	return rotY_RPN->eval();
}

float LanWaveObj::getRotZ()
{
	return rotZ_RPN->eval();
}

float LanWaveObj::getColR()
{
	return colR_RPN->eval();
}

float LanWaveObj::getColG()
{
	return colG_RPN->eval();
}

float LanWaveObj::getColB()
{
	return colB_RPN->eval();
}

int LanWaveObj::getModelID()
{
	return modelID;
}

bool LanWaveObj::isWireframe()
{
	return wireframe;
}