#pragma once

#include <iostream>
#include <string>

#include "LanWaveRPN.hpp"

class LanWaveObj
{
	public:
		LanWaveObj(std::string modelName);
		~LanWaveObj();

		//void setInfoPackage( const visualiser_information *infoPackage);

		std::string getModelName();

		void setModelID(int id);
		int getModelID();

		void givePosX(std::string input);
		void givePosY(std::string input);
		void givePosZ(std::string input);

		void giveSclX(std::string input);
		void giveSclY(std::string input);
		void giveSclZ(std::string input);

		void giveRotX(std::string input);
		void giveRotY(std::string input);
		void giveRotZ(std::string input);

		void giveColR(std::string input);
		void giveColG(std::string input);
		void giveColB(std::string input);

		void setWireframe(bool state);

		float getPosX();
		float getPosY();
		float getPosZ();

		float getSclX();
		float getSclY();
		float getSclZ();

		float getRotX();
		float getRotY();
		float getRotZ();

		float getColR();
		float getColG();
		float getColB();

		bool isWireframe();

	private:
		std::string model_name;


		LanWaveRPN *posX_RPN;
		LanWaveRPN *posY_RPN;
		LanWaveRPN *posZ_RPN;

		LanWaveRPN *sclX_RPN;
		LanWaveRPN *sclY_RPN;
		LanWaveRPN *sclZ_RPN;

		LanWaveRPN *rotX_RPN;
		LanWaveRPN *rotY_RPN;
		LanWaveRPN *rotZ_RPN;

		LanWaveRPN *colR_RPN;
		LanWaveRPN *colG_RPN;
		LanWaveRPN *colB_RPN;

		bool wireframe;
		int modelID;

};