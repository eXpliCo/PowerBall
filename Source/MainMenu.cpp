#include "MainMenu.h"

MainMenu::MainMenu(GraphicsEngine* ge)
{
	this->mNrOfSets = 30;
	this->mSets = new GUISet[this->mNrOfSets]();
	this->mRunning = true;
	this->mCurrentSet = MAINMENU;
	this->mSubSet = NOMENU;
	this->mGm = NULL;
	this->mGe = ge;
	
	this->Initialize();
}
MainMenu::~MainMenu()
{
	if(this->mSets)
		delete [] this->mSets;
}
bool MainMenu::Initialize()
{
	CreateCircleMenu();

	CreateOptionsMenu();
	
	CreateHotseatMenu();
	
	CreateOnlineAndLanMenu();

	mGe->LoadingScreen("Media/LoadingScreen/LoadingScreenBG.png", "Media/LoadingScreen/LoadingScreenPB.png");
	
	return true;
}
void MainMenu::UpdateMousePosition()
{
	float windowWidth = (float)this->mGe->GetEngineParameters().windowWidth;
	float windowHeight = (float)this->mGe->GetEngineParameters().windowHeight;
	float lengthFromMiddle = (windowHeight * 0.745f) / 3;
	D3DXVECTOR2 mouseVector = this->mGe->GetKeyListener()->GetMousePosition();
	D3DXVECTOR2 centerVector = D3DXVECTOR2(windowWidth / 2, windowHeight / 2);
	D3DXVECTOR2 centerToMouseVec = (centerVector - mouseVector);

	if(D3DXVec2Length(&centerToMouseVec) > lengthFromMiddle+1 || D3DXVec2Length(&centerToMouseVec) < lengthFromMiddle-1)
	{
		centerToMouseVec = (lengthFromMiddle / D3DXVec2Length(&centerToMouseVec)) * centerToMouseVec;
		this->mGe->GetKeyListener()->SetMousePosition(centerVector + (centerToMouseVec * (-1)));
	}
	
}

bool MainMenu::Run()
{
	this->mSets[MAINMENU].AddSetToRenderer(this->mGe);
	bool IsClicked = false;

	CursorControl cc;
	cc.SetVisibility(false);

	float dt;
	float updateMouse = 50;
	this->mGe->GetKeyListener()->SetMousePosition(D3DXVECTOR2((float)this->mGe->GetEngineParameters().windowWidth / 2, (float)this->mGe->GetEngineParameters().windowHeight / 3));
	GUIEvent *returnEvent = NULL;
	bool menuChange = false;
	bool mousePressed = false;
	bool isRunning = true;
	float menuChangeTime = 0;
	while(isRunning)
	{
		
		returnEvent = NULL;
		dt = this->mGe->Update();

		IsClicked = this->mGe->GetKeyListener()->IsClicked(1);

		if(this->mCurrentSet == MAINMENU || this->mCurrentSet == MAINMENU_PLAY)
		{
			this->KeyBoardSteering(IsClicked);

			CursorControl cc;
			cc.SetVisibility(false);

			/*Mouse Update*/
			if(updateMouse < 0)
			{
				if(GetForegroundWindow() == this->mGe->GetWindowHandle())
					this->UpdateMousePosition();
				
				updateMouse = 50;
			}
			else{ updateMouse -= dt; }
		}
		else if(this->mCurrentSet == OPTIONS_GAMEPLAY || this->mCurrentSet == OPTIONS_HOTSEAT || this->mCurrentSet == OPTIONS_ONLINE
			|| this->mCurrentSet == OPTIONS_LAN)
		{
			CursorControl cc;
			cc.SetVisibility(true);
		}

		/*If mouse is clicked*/
		if(IsClicked && !mousePressed)
		{
			mousePressed = true;
		}

		if(!menuChange)
		{
			D3DXVECTOR2 mousePos = this->mGe->GetKeyListener()->GetMousePosition();
			returnEvent = this->mSets[this->mCurrentSet].UpdateAndCheckCollision(mousePos.x, mousePos.y, IsClicked, this->mGe);
			if(returnEvent == NULL && this->mSubSet != NOMENU)
				returnEvent = this->mSets[this->mSubSet].UpdateAndCheckCollision(mousePos.x, mousePos.y, IsClicked, this->mGe);
			if(returnEvent != NULL)
			{
				ChangeSetEvent* tempReturnEvent1 = (ChangeSetEvent*)returnEvent;
				if(returnEvent->GetEventMessage() == "ChangeSetEvent")
				{
					ChangeSetEvent* tempReturnEvent = (ChangeSetEvent*)returnEvent;
					int tempEventSet = tempReturnEvent->GetSet();
					if(tempEventSet == PLAY_LAN)
					{
						CursorControl cc;
						cc.SetVisibility(true);
						int lifes = -1;
						int rounds = -1; 
						int flags = -1;
						int secondsToWin = -1;

						/* Getting some needed info to start a server*/
						GUIEvent* tempEvent = this->mSets[this->mSubSet].GetEventFromDropDown("GameMode");
						ChangeSubSetEvent* GameMode = (ChangeSubSetEvent*) tempEvent;
						if(GameMode->GetGameMode() == CTF)
						{
							flags = atoi(this->mSets[this->mSubSet].GetTextFromField("Flags").c_str());
						}
						else if(GameMode->GetGameMode() == KOTH)
						{
							secondsToWin = atoi(this->mSets[this->mSubSet].GetTextFromField("SecondsToWin").c_str());
							rounds = atoi(this->mSets[this->mSubSet].GetTextFromField("Rounds").c_str());
						}
						else if(GameMode->GetGameMode() == WARLOCK)
						{

						}

						this->mGm = new GameManager(this->mGe);
						
						//add button called find servers or something and copy the row below to retrieve a list of all servers currently open on the LAN
						vector<ServerInfo> servers = this->mGm->GetLanPointer()->FindServers();
						int chosenServer = 0; //index of the server selected from the list

						this->mSets[this->mCurrentSet].RemoveSetFromRenderer(this->mGe);
						this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);
						if(servers.size() > 0)
						{
							this->mGm->PlayLAN(servers[chosenServer]);
						}
						else //atm, will host if no servers running on LAN
						{
							ServerInfo host("PowerBall Server", 0, 5, GameMode->GetGameMode(), "");
							this->mGm->PlayLAN(host);
						} 
						
						this->mSubSet = NOMENU;
						this->mCurrentSet = MAINMENU;

						SAFE_DELETE(this->mGm);
					}
					if(tempEventSet == PLAY_ONLINE)
					{
						/*CursorControl cc;
						cc.SetVisibility(true);
						this->mGm = new GameManager(this->mGe);
						
						//this->mGm->PlayONLINE("", CTF); //Change "" to "the servers ip" if you want to connect to a server.  (if you want to be host leave it blank)
						this->mGm->Play(2);
						
						SAFE_DELETE(this->mGm);*/
					}
					if(tempEventSet == MAINMENU_PLAY)
					{
						this->mSets[this->mCurrentSet].RemoveSetFromRenderer(this->mGe);
						this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);
						this->mSubSet = NOMENU;
						this->mCurrentSet = MAINMENU_PLAY;
					}
					if(tempEventSet == PLAY_HOTSEAT)
					{
						this->mSubSet = NOMENU;
						CursorControl cc;
						cc.SetVisibility(true);
						string lifes = this->mSets[this->mCurrentSet].GetTextFromField("Lifes");
						string rounds = this->mSets[this->mCurrentSet].GetTextFromField("Rounds");

						this->mGm = new GameManager(this->mGe);

						this->mSets[this->mCurrentSet].RemoveSetFromRenderer(this->mGe);
						this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);

						this->mGm->Play(2, atoi(lifes.c_str()), atoi(rounds.c_str()));

						this->mSubSet = NOMENU;
						this->mCurrentSet = MAINMENU;

						SAFE_DELETE(this->mGm);
						
					}
					else if(tempEventSet == EXIT)
					{
						this->mSubSet = NOMENU;
						this->mSets[this->mCurrentSet].RemoveSetFromRenderer(this->mGe);
						this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);
						SAFE_DELETE(this->mGm);
						isRunning = false;
						return true;
					}
					else if(tempEventSet == OPTIONS_GAMEPLAY)
					{
						this->mSets[this->mCurrentSet].RemoveSetFromRenderer(this->mGe);
						this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);
						this->mSubSet = NOMENU;
						this->mCurrentSet = OPTIONS_GAMEPLAY;
					}
					else if(tempEventSet == MAINMENU)
					{
						this->mSets[this->mCurrentSet].RemoveSetFromRenderer(this->mGe);
						this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);
						this->mSubSet = NOMENU;
						this->mCurrentSet = MAINMENU;
					}
					else if(tempEventSet == OPTIONS_HOTSEAT)
					{
						this->mSets[this->mCurrentSet].RemoveSetFromRenderer(this->mGe);
						this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);
						this->mSubSet = NOMENU;
						this->mCurrentSet = OPTIONS_HOTSEAT;
					}
					else if(tempEventSet == OPTIONS_ONLINE)
					{
						this->mSets[this->mCurrentSet].RemoveSetFromRenderer(this->mGe);
						this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);
						this->mSubSet = NOMENU;
						this->mCurrentSet = OPTIONS_ONLINE;
					}
					else if(tempEventSet == OPTIONS_LAN)
					{
						this->mSets[this->mCurrentSet].RemoveSetFromRenderer(this->mGe);
						this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);
						this->mSubSet = OPTIONS_LAN_SUBCTF;
						this->mCurrentSet = OPTIONS_LAN;

						float windowWidth = (float)this->mGe->GetEngineParameters().windowWidth;
						float windowHeight = (float)this->mGe->GetEngineParameters().windowHeight;
						float dx = (windowHeight * 4.0f) / 3.0f;
						float offSet = (windowWidth - dx) / 2.0f;

						/*this->mGm = new GameManager(this->mGe);
						//add button called find servers or something and copy the row below to retrieve a list of all servers currently open on the LAN
						vector<ServerInfo> servers = this->mGm->GetLanPointer()->FindServers();
						Element* tempElement;
						for(int i = 0; i < servers.size(); i++)
						{
							tempElement = new TextBox(dx * (680.0f / 1200.0f), windowHeight * (280.0f / 900.0f) + 40 * i, 1, "Media/Menus/EmptyMenu.png", 0, 0, servers.at(i).GetServerName(), "Server" + MaloW::convertNrToString(i), 0.80, servers.at(i).GetServerName().size());
							this->mSets[this->mSubSet].AddElement(tempElement);
						}
						tempElement = NULL;
						SAFE_DELETE(this->mGm);*/
					}

					this->mSets[this->mCurrentSet].AddSetToRenderer(this->mGe);
					this->mSets[this->mSubSet].AddSetToRenderer(this->mGe);
					menuChange = true;
					menuChangeTime = 50;
					
				}
				else if(returnEvent->GetEventMessage() == "ChangeResEvent")
				{
					ChangeResEvent* tempReturnEvent = (ChangeResEvent*)returnEvent;
					int width = 0, height = 0;
					width = tempReturnEvent->GetWidth();
					height = tempReturnEvent->GetHeight();
					/*
					Make something that change res here
					*/
				}
				if(returnEvent->GetEventMessage() == "ChangeSubSetEvent")
				{
					this->mSets[this->mSubSet].RemoveSetFromRenderer(this->mGe);

					ChangeSubSetEvent* tempReturnEvent = (ChangeSubSetEvent*)returnEvent;
					int tempEventSet = tempReturnEvent->GetSet();

					int set = tempReturnEvent->GetSet();
					this->mSubSet = set;
					this->mSets[this->mSubSet].AddSetToRenderer(this->mGe);
					menuChange = true;
					menuChangeTime = 50;
				}
			}
		}
		/*If mouse is not clicked*/
		if(!IsClicked && mousePressed)
		{
			mousePressed = false;
		}
		if(!this->mGe->isRunning())
			isRunning = false;
		else
		{
			menuChangeTime = menuChangeTime - dt;
			
			if(menuChangeTime < 0)
			{
				menuChange = false;
			}
		}
	}
	
	return true;
}

void MainMenu::KeyBoardSteering(bool& IsClicked)
{

	float windowWidth = (float)this->mGe->GetEngineParameters().windowWidth;
	float windowHeight = (float)this->mGe->GetEngineParameters().windowHeight;
	if(this->mGe->GetKeyListener()->IsPressed(VK_RETURN))
		IsClicked = true;

	if(this->mGe->GetKeyListener()->IsPressed(VK_UP) || this->mGe->GetKeyListener()->IsPressed('W'))
		this->mGe->GetKeyListener()->SetMousePosition(D3DXVECTOR2( (windowWidth/2), (windowHeight/4)));
	else if(this->mGe->GetKeyListener()->IsPressed(VK_LEFT) || this->mGe->GetKeyListener()->IsPressed('A'))
		this->mGe->GetKeyListener()->SetMousePosition(D3DXVECTOR2( (windowWidth/4), (windowHeight/2)));
	else if(this->mGe->GetKeyListener()->IsPressed(VK_DOWN) || this->mGe->GetKeyListener()->IsPressed('S'))
		this->mGe->GetKeyListener()->SetMousePosition(D3DXVECTOR2( (windowWidth/2), (windowHeight/4)*3));
	else if(this->mGe->GetKeyListener()->IsPressed(VK_RIGHT) || this->mGe->GetKeyListener()->IsPressed('D'))
		this->mGe->GetKeyListener()->SetMousePosition(D3DXVECTOR2( (windowWidth/4)*3, (windowHeight/2)));
}