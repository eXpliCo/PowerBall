#ifndef MALOWENGINEPARAMS
#define MALOWENGINEPARAMS

#include "stdafx.h"
#include <fstream>

enum CameraType
{
	FPS = 0,
	RTS = 1,
	TRD = 2,
};

class GraphicsEngineParams
{
public:
	static int windowWidth;
	static int windowHeight;
	static int ShadowMapSettings;
	static int FXAAQuality;
	static CameraType CamType;

	GraphicsEngineParams()
	{

	}

	void LoadFromeFile(string file)
	{
		ifstream in;
		in.open(file.c_str());
		if(!in)
		{
			MaloW::Debug("Failed to load EngineParameters from " + file + ". Might be because it didn't excist, creating it now.");
			this->SaveToFile(file);
			return;
		}

		string line = "";

		getline(in, line);
		this->windowWidth = atoi(line.c_str());
		getline(in, line);
		this->windowHeight = atoi(line.c_str());
		getline(in, line);
		this->ShadowMapSettings = atoi(line.c_str());
		getline(in, line);
		this->FXAAQuality = atoi(line.c_str());


		in.close();
	}

	void SaveToFile(string file)
	{
		ofstream out;
		out.open(file);
		if(!out)
		{
			MaloW::Debug("Failed to save EngineParameters from " + file);
			return;
		}

		out << this->windowWidth << endl;
		out << this->windowHeight << endl;
		out << this->ShadowMapSettings << endl;
		out << this->FXAAQuality << endl;

		out.close();
	}

	virtual ~GraphicsEngineParams() { }
};

#endif