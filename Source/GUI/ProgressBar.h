#pragma once 
/**
* Author: Jerry Rahmqvist
* Create Date: 16/05/2012
* 
* This Class is a ProgressBar that is scaling with help of one background image and
* two bar images. You can select how much percentage boarder you want around the bar. 
* You can select different boardersize on the top/bottom and right/left. 
* The percentage is calculated from the backgrounds point of view. 
*
* Usage: 
*		ProgressBar* temp = new ProgressBar();		// will give you a bar around top and center (start from 40% of the width)
*		temp->SetPercentOfProgressBarColor1(50);	// give you 50% bar from original size and scalex along x-axis
**/
#include "GraphicsEngine.h"


class ProgressBar
{
	private:
		float mHeight;
		float mWidth;
		
		float mStartBgX;
		float mStartBgY;
		float mFullX;
		float mFullY;

		float mProcentBoarderX;
		float mProcentBoarderY;
		float mStartBarX;
		float mStartBarY;
		float mFullBarX;
		float mFullBarY;

		Image* mBackground;
		Image* mBarColor1;
		Image* mBarColor2;
	public:
		
		ProgressBar(std::string bar = "Media/LoadingScreen/DarkBlue.png", std::string bar2 = "Media/LoadingScreen/Red.png" , std::string background = "Media/LoadingScreen/Black.png", 
						D3DXVECTOR2 positionPercentageTopLeft = D3DXVECTOR2(0.4f, 0.03f),
						float percentageSizeOfWindowX = 0.2f, float percentageSizeOfWindowY = 0.05f, 	
						float percentageBoarderOfBarX = 0.01f, float percentageBoarderOfBarY = 0.05f);
		ProgressBar(D3DXVECTOR2 positionPercentageTopLeft);
		~ProgressBar();
		void SetPercentOfProgressBarColor1(float procentageX);
		void SetPercentOfProgressBarColor2(float procentageX);
		void ResetBar();
};